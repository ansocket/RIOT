/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_ch32v30x
 * @ingroup     drivers_periph_i2c
 * @brief       Low-level I2C driver for CH32V30x
 *
 * Writes support I2C_NOSTOP/I2C_NOSTART, including repeated START register
 * reads. Split reads are not supported: the two-byte receive pipeline needs
 * the final transfer length before clearing ADDR to schedule the last NACK.
 */

#include <assert.h>
#include <errno.h>

#include "irq.h"
#include "mutex.h"
#include "periph/i2c.h"
#include "periph/gpio.h"
#include "periph_conf.h"
#include "periph_cpu.h"
#include "ztimer.h"

#define I2C_TIMEOUT_US          (100000U)

#define CTLR1_PE                (1UL << 0)
#define CTLR1_START             (1UL << 8)
#define CTLR1_STOP              (1UL << 9)
#define CTLR1_ACK               (1UL << 10)
#define CTLR1_POS               (1UL << 11)
#define CTLR1_SWRST             (1UL << 15)
#define CTLR2_FREQ_MAX          (60U)
#define STAR1_SB                (1UL << 0)
#define STAR1_ADDR              (1UL << 1)
#define STAR1_BTF               (1UL << 2)
#define STAR1_ADD10             (1UL << 3)
#define STAR1_RXNE              (1UL << 6)
#define STAR1_TXE               (1UL << 7)
#define STAR1_BERR              (1UL << 8)
#define STAR1_ARLO              (1UL << 9)
#define STAR1_AF                (1UL << 10)
#define STAR1_OVR               (1UL << 11)
#define STAR2_MSL               (1UL << 0)
#define STAR2_BUSY              (1UL << 1)
#define STAR2_TRA               (1UL << 2)
#define CKCFGR_FAST             (1UL << 15)
#define CKCFGR_CCR_MASK         (0xfffUL)

#define I2C_ERRORS              (STAR1_BERR | STAR1_ARLO | STAR1_AF | STAR1_OVR)

static mutex_t _locks[I2C_NUMOF];

static inline void _clear_errors(I2C_TypeDef *dev)
{
    /* CH32V30x clears these status bits by writing zero. */
    dev->STAR1 = (uint32_t)~I2C_ERRORS;
}

static int _wait(I2C_TypeDef *dev, uint32_t mask)
{
    uint32_t start = ztimer_now(ZTIMER_USEC);

    while (1) {
        uint32_t status = dev->STAR1;
        /* Check errors first, even if an event flag is set as well. */
        if (status & STAR1_ARLO) {
            return -EAGAIN;
        }
        if (status & (STAR1_BERR | STAR1_AF | STAR1_OVR)) {
            return -EIO;
        }
        if ((status & mask) == mask) {
            return 0;
        }
        if ((uint32_t)(ztimer_now(ZTIMER_USEC) - start) > I2C_TIMEOUT_US) {
            return -ETIMEDOUT;
        }
    }
}

static int _wait_idle(I2C_TypeDef *dev)
{
    uint32_t start = ztimer_now(ZTIMER_USEC);

    while ((dev->CTLR1 & CTLR1_STOP) || (dev->STAR2 & STAR2_BUSY)) {
        if ((uint32_t)(ztimer_now(ZTIMER_USEC) - start) >= I2C_TIMEOUT_US) {
            return -ETIMEDOUT;
        }
    }
    return 0;
}

static int _stop(I2C_TypeDef *dev)
{
    /* Never generate STOP after losing arbitration to another master. */
    if (dev->STAR2 & STAR2_MSL) {
        dev->CTLR1 &= ~CTLR1_ACK;
        dev->CTLR1 |= CTLR1_STOP;
    }
    return _wait_idle(dev);
}

static void _clear_addr(I2C_TypeDef *dev)
{
    /* This sequence releases SCL and starts the data phase. */
    (void)dev->STAR1;
    (void)dev->STAR2;
}

static int _send_start(I2C_TypeDef *dev)
{
    dev->CTLR1 |= CTLR1_START;
    return _wait(dev, STAR1_SB);
}

static int _wait_addr(I2C_TypeDef *dev, uint32_t mask)
{
    int res = _wait(dev, mask);

    /* Only a NACK in the address phase means that no device responded. */
    if (res == -EIO && (dev->STAR1 & STAR1_AF) &&
        !(dev->STAR1 & (STAR1_BERR | STAR1_OVR))) {
        return -ENXIO;
    }
    return res;
}

static int _send_address(I2C_TypeDef *dev, uint16_t addr, bool read,
                         bool addr10)
{
    if (addr10) {
        /* A 10-bit transfer always starts with the write header. */
        dev->DATAR = 0xf0 | ((addr >> 7) & 0x06);
        int res = _wait_addr(dev, STAR1_ADD10);
        if (res < 0) {
            return res;
        }
        dev->DATAR = addr & 0xff;
        res = _wait_addr(dev, STAR1_ADDR);
        if (res < 0) {
            return res;
        }
        if (read) {
            _clear_addr(dev);
            res = _send_start(dev);
            if (res < 0) {
                return res;
            }
            dev->DATAR = 0xf1 | ((addr >> 7) & 0x06);
            res = _wait_addr(dev, STAR1_ADDR);
            if (res < 0) {
                return res;
            }
        }
    }
    else {
        dev->DATAR = (addr << 1) | (read ? 1 : 0);
        int res = _wait_addr(dev, STAR1_ADDR);
        if (res < 0) {
            return res;
        }
    }

    /* Leave ADDR set so the caller can configure ACK/POS before reception. */
    return 0;
}

static int _transfer_start(i2c_t dev, uint16_t addr, bool read, uint8_t flags)
{
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    if (flags & I2C_NOSTART) {
        uint32_t status = i2c->STAR2;
        return ((status & (STAR2_MSL | STAR2_TRA)) == (STAR2_MSL | STAR2_TRA))
               ? 0 : -EOPNOTSUPP;
    }
    /* A busy bus is allowed only for our own repeated START. */
    if (!(i2c->STAR2 & STAR2_MSL)) {
        int res = _wait_idle(i2c);
        if (res < 0) {
            return res;
        }
    }
    _clear_errors(i2c);
    i2c->CTLR1 = (i2c->CTLR1 & ~CTLR1_POS) | CTLR1_ACK;
    int res = _send_start(i2c);
    if (res == 0) {
        res = _send_address(i2c, addr, read, flags & I2C_ADDR10);
    }
    return res;
}

void i2c_init_pins(i2c_t dev)
{
    assert(dev < I2C_NUMOF);
    gpio_init_af(i2c_config[dev].scl_pin, GPIO_AF_OUT_OD);
    gpio_init_af(i2c_config[dev].sda_pin, GPIO_AF_OUT_OD);
}

void i2c_deinit_pins(i2c_t dev)
{
    assert(dev < I2C_NUMOF);
    /* Set the output latches BEFORE switching to GPIO, avoiding low pulses. */
    periph_clk_en(APB2, (RCC_APB2PCENR_IOPAEN << (i2c_config[dev].scl_pin >> 4)) |
                       (RCC_APB2PCENR_IOPAEN << (i2c_config[dev].sda_pin >> 4)));
    gpio_set(i2c_config[dev].scl_pin);
    gpio_set(i2c_config[dev].sda_pin);
    gpio_init(i2c_config[dev].scl_pin, GPIO_OD);
    gpio_init(i2c_config[dev].sda_pin, GPIO_OD);
}

static void _configure(i2c_t dev)
{
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    uint32_t clk = periph_apb_clk(i2c_config[dev].bus);
    uint32_t freq = clk / MHZ(1);
    uint32_t speed = i2c_config[dev].speed;

    assert(freq >= 2);
    assert(speed == I2C_SPEED_LOW || speed == I2C_SPEED_NORMAL ||
           speed == I2C_SPEED_FAST || speed == I2C_SPEED_FAST_PLUS);
    /* WCH limits FREQ to 60 even when PCLK1 runs at 72 MHz. */
    if (freq > CTLR2_FREQ_MAX) {
        freq = CTLR2_FREQ_MAX;
    }

    i2c->CTLR1 = CTLR1_SWRST;
    i2c->CTLR1 = 0;
    i2c->CTLR2 = freq;
    uint32_t divider;
    if (speed > 100000U) {
        /* Round up so SCL never exceeds the requested speed (duty cycle 2). */
        divider = (clk + 3 * speed - 1) / (3 * speed);
        i2c->RTR = (freq * 300) / 1000 + 1;
    }
    else {
        divider = (clk + 2 * speed - 1) / (2 * speed);
        if (divider < 4) {
            divider = 4;
        }
        i2c->RTR = freq + 1;
    }
    assert(divider && divider <= CKCFGR_CCR_MASK);
    i2c->CKCFGR = divider | ((speed > 100000U) ? CKCFGR_FAST : 0);
    _clear_errors(i2c);
}

static void _recover(i2c_t dev)
{
    i2c_config[dev].dev->CTLR1 = 0;
    i2c_deinit_pins(dev);
    _configure(dev);
    i2c_config[dev].dev->CTLR1 = CTLR1_PE | CTLR1_ACK;
    i2c_init_pins(dev);
}

static int _abort(i2c_t dev, int res)
{
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    _clear_errors(i2c);
    /* Do not wait for, or disturb, the winning master's transaction. */
    if (res == -EAGAIN) {
        return res;
    }
    int stop_res = _stop(i2c);
    if (res == -ETIMEDOUT || stop_res < 0) {
        _recover(dev);
    }
    return res;
}

void i2c_init(i2c_t dev)
{
    assert(dev < I2C_NUMOF);
    mutex_init(&_locks[dev]);
    periph_clk_en(i2c_config[dev].bus, i2c_config[dev].rcc_mask);
    i2c_deinit_pins(dev);
    _configure(dev);
    /* Remain powered off until i2c_acquire(). */
    periph_clk_dis(i2c_config[dev].bus, i2c_config[dev].rcc_mask);
}

void i2c_acquire(i2c_t dev)
{
    assert(dev < I2C_NUMOF);
    mutex_lock(&_locks[dev]);
    periph_clk_en(i2c_config[dev].bus, i2c_config[dev].rcc_mask);
    i2c_init_pins(dev);
    i2c_config[dev].dev->CTLR1 |= CTLR1_PE | CTLR1_ACK;
}

void i2c_release(i2c_t dev)
{
    assert(dev < I2C_NUMOF);
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    if (_stop(i2c) < 0) {
        /* STOP could not finish: reset the controller before gating its clock. */
        _recover(dev);
    }
    i2c_config[dev].dev->CTLR1 &= ~CTLR1_PE;
    i2c_deinit_pins(dev);
    periph_clk_dis(i2c_config[dev].bus, i2c_config[dev].rcc_mask);
    mutex_unlock(&_locks[dev]);
}

int i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data, size_t len,
                    uint8_t flags)
{
    assert(dev < I2C_NUMOF && data && len);
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    int res = _transfer_start(dev, addr, false, flags);
    if (res < 0) {
        return _abort(dev, res);
    }
    if (!(flags & I2C_NOSTART)) {
        _clear_addr(i2c);
    }
    const uint8_t *buf = data;
    for (size_t i = 0; i < len; i++) {
        if ((res = _wait(i2c, STAR1_TXE)) < 0) {
            return _abort(dev, res);
        }
        i2c->DATAR = buf[i];
        if ((res = _wait(i2c, STAR1_BTF)) < 0) {
            return _abort(dev, res);
        }
    }
    if (!(flags & I2C_NOSTOP)) {
        res = _stop(i2c);
        if (res < 0) {
            return _abort(dev, res);
        }
    }
    return 0;
}

int i2c_read_bytes(i2c_t dev, uint16_t addr, void *data, size_t len,
                   uint8_t flags)
{
    assert(dev < I2C_NUMOF && data && len);
    if (flags & (I2C_NOSTART | I2C_NOSTOP)) {
        return -EOPNOTSUPP;
    }
    I2C_TypeDef *i2c = i2c_config[dev].dev;
    int res = _transfer_start(dev, addr, true, flags);
    if (res < 0) {
        return _abort(dev, res);
    }
    uint8_t *buf = data;
    if (len == 1) {
        /* EV6_3: NACK the only byte and request STOP before it arrives. */
        unsigned state = irq_disable();
        i2c->CTLR1 &= ~CTLR1_ACK;
        _clear_addr(i2c);
        i2c->CTLR1 |= CTLR1_STOP;
        irq_restore(state);
        if ((res = _wait(i2c, STAR1_RXNE)) < 0) {
            return _abort(dev, res);
        }
        *buf = i2c->DATAR;
    }
    else if (len == 2) {
        /* POS moves the NACK to the second byte. ADDR holds SCL until ready. */
        unsigned state = irq_disable();
        i2c->CTLR1 |= CTLR1_POS;
        _clear_addr(i2c);
        i2c->CTLR1 &= ~CTLR1_ACK;
        irq_restore(state);
        if ((res = _wait(i2c, STAR1_BTF)) < 0) {
            return _abort(dev, res);
        }
        state = irq_disable();
        i2c->CTLR1 |= CTLR1_STOP;
        *buf++ = i2c->DATAR;
        *buf = i2c->DATAR;
        irq_restore(state);
    }
    else {
        _clear_addr(i2c);
        while (len > 3) {
            if ((res = _wait(i2c, STAR1_RXNE)) < 0) {
                return _abort(dev, res);
            }
            *buf++ = i2c->DATAR;
            len--;
        }
        /* BTF stretches SCL with two bytes buffered. NACK the final byte
         * before reading the third-last byte and allowing reception to resume. */
        if ((res = _wait(i2c, STAR1_BTF)) < 0) {
            return _abort(dev, res);
        }
        unsigned state = irq_disable();
        i2c->CTLR1 &= ~CTLR1_ACK;
        *buf++ = i2c->DATAR;
        irq_restore(state);
        if ((res = _wait(i2c, STAR1_BTF)) < 0) {
            return _abort(dev, res);
        }
        state = irq_disable();
        i2c->CTLR1 |= CTLR1_STOP;
        *buf++ = i2c->DATAR;
        *buf = i2c->DATAR;
        irq_restore(state);
    }
    res = _wait_idle(i2c);
    if (res < 0) {
        return _abort(dev, res);
    }
    i2c->CTLR1 &= ~CTLR1_POS;
    return 0;
}
