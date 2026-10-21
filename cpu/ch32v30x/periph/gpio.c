/*
 * SPDX-FileCopyrightText: 2014-2015 Freie Universität Berlin
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-FileCopyrightText: 2023 Gunar Schorcht <gunar@schorcht.net>
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup         cpu_ch32v30x
 * @{
 *
 * @file
 * @brief           Low-level GPIO driver implementation for CH32V30x
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author          Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author          Gunar Schorcht
 * @author          ansocket
 */

#include <assert.h>

#include "bitarithm.h"
#include "cpu.h"
#include "log.h"
#include "periph_cpu.h"
#include "periph/gpio.h"
#include "pfic.h"

/**
 * @brief   Extract information from mode parameter
 */
#define MODE_MASK                       (0x0f)
#define ODR_POS                         (4U)

#ifdef MODULE_PERIPH_GPIO_IRQ
/**
 * @brief   Number of available external interrupt lines
 */
#define GPIO_ISR_CHAN_NUMOF             (16U)
#define GPIO_ISR_CHAN_MASK              (0xFFFF)

/**
 * @brief   Allocate memory for one callback and argument per EXTI channel
 */
static gpio_isr_ctx_t exti_ctx[GPIO_ISR_CHAN_NUMOF];
#endif /* MODULE_PERIPH_GPIO_IRQ */

static inline unsigned _port_num(gpio_t pin)
{
    return pin >> 4;
}

static inline unsigned _pin_num(gpio_t pin)
{
    return pin & 0x0f;
}

static inline GPIO_TypeDef *_port(gpio_t pin)
{
    return (GPIO_TypeDef *)(GPIOA_BASE + _port_num(pin) * GPIO_PORT_OFFSET);
}

static inline void _port_enable_clock(gpio_t pin)
{
    periph_clk_en(APB2, RCC_APB2PCENR_IOPAEN << _port_num(pin));
}

/**
 * @brief   Check if the given mode is some kind of input mode
 */
static inline bool _mode_is_input(gpio_mode_t mode)
{
    return !(mode & 0x3);
}

static inline void _set_cfg(GPIO_TypeDef *port, unsigned pin_num, unsigned cfg)
{
    volatile uint32_t *cfgr = (pin_num < 8) ? &port->CFGLR : &port->CFGHR;
    unsigned shift = (pin_num & 0x7) * 4;

    *cfgr = (*cfgr & ~(0xfUL << shift)) | ((uint32_t)(cfg & MODE_MASK) << shift);
}

static inline bool _pin_is_output(GPIO_TypeDef *port, unsigned pin_num)
{
    uint32_t cfgr = (pin_num < 8) ? port->CFGLR : port->CFGHR;

    /* MODE bits != 0 means output */
    return cfgr & (0x3UL << ((pin_num & 0x7) * 4));
}

int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    GPIO_TypeDef *port = _port(pin);
    unsigned pin_num = _pin_num(pin);

    /* open-drain output with pull-up is not supported */
    if (mode == GPIO_OD_PU) {
        return -1;
    }

    _port_enable_clock(pin);

    /* for inputs, OUTDR selects pull-up or pull-down, so set it first */
    if (_mode_is_input(mode)) {
        if (mode & (1U << ODR_POS)) {
            port->BSHR = 1UL << pin_num;
        }
        else {
            port->BCR = 1UL << pin_num;
        }
    }

    _set_cfg(port, pin_num, mode);

    return 0;
}

void gpio_init_analog(gpio_t pin)
{
    _port_enable_clock(pin);
    /* CNF = 0, MODE = 0: analog input */
    _set_cfg(_port(pin), _pin_num(pin), 0);
}

bool gpio_read(gpio_t pin)
{
    GPIO_TypeDef *port = _port(pin);
    unsigned pin_num = _pin_num(pin);

    if (_pin_is_output(port, pin_num)) {
        return port->OUTDR & (1UL << pin_num);
    }
    return port->INDR & (1UL << pin_num);
}

void gpio_set(gpio_t pin)
{
    _port(pin)->BSHR = 1UL << _pin_num(pin);
}

void gpio_clear(gpio_t pin)
{
    _port(pin)->BCR = 1UL << _pin_num(pin);
}

void gpio_toggle(gpio_t pin)
{
    GPIO_TypeDef *port = _port(pin);
    uint32_t mask = 1UL << _pin_num(pin);

    /* upper half of BSHR resets the pin, lower half sets it */
    port->BSHR = (port->OUTDR & mask) ? (mask << 16) : mask;
}

void gpio_write(gpio_t pin, bool value)
{
    if (value) {
        gpio_set(pin);
    }
    else {
        gpio_clear(pin);
    }
}

#ifdef MODULE_PERIPH_GPIO_IRQ

static void _gpio_isr(unsigned irqn);

static inline unsigned _irq_num(unsigned pin_num)
{
    if (pin_num < 5) {
        return EXTI0_IRQn + pin_num;
    }
    if (pin_num < 10) {
        return EXTI9_5_IRQn;
    }
    return EXTI15_10_IRQn;
}

#ifndef NDEBUG
static uint8_t exti_line_port[GPIO_ISR_CHAN_NUMOF];
#endif

int gpio_init_int(gpio_t pin, gpio_mode_t mode, gpio_flank_t flank,
                  gpio_cb_t cb, void *arg)
{
    assert(cb != NULL);

    unsigned pin_num = _pin_num(pin);
    unsigned port_num = _port_num(pin);

    /* disable interrupts on the channel we want to edit (just in case) */
    EXTI->INTENR &= ~(1UL << pin_num);

    if (gpio_init(pin, mode) != 0) {
        return -1;
    }

#ifndef NDEBUG
    /* all ports share the same EXTI line for the same pin number, i.e.
     * PA<n> shares the EXTI line with PB<n>, PC<n>, PD<n> and PE<n> */
    if ((exti_ctx[pin_num].cb != NULL) && (exti_line_port[pin_num] != port_num)) {
        LOG_ERROR("EXTI line for GPIO_PIN(%u, %u) is used by GPIO_PIN(%u, %u).\n",
                  port_num, pin_num, exti_line_port[pin_num], pin_num);
        assert(0);
    }
    exti_line_port[pin_num] = port_num;
#endif

    exti_ctx[pin_num].cb = cb;
    exti_ctx[pin_num].arg = arg;

    /* route the pin to its EXTI line */
    periph_clk_en(APB2, RCC_APB2PCENR_AFIOEN);
    volatile uint32_t *exticr = &AFIO->EXTICR[pin_num >> 2];
    unsigned shift = (pin_num & 0x3) * 4;
    *exticr = (*exticr & ~(0xfUL << shift)) | ((uint32_t)port_num << shift);

    /* configure the active flank */
    EXTI->RTENR = (EXTI->RTENR & ~(1UL << pin_num)) | ((flank & 0x1UL) << pin_num);
    EXTI->FTENR = (EXTI->FTENR & ~(1UL << pin_num)) | (((flank >> 1) & 0x1UL) << pin_num);

    /* clear any pending request (write 1 to clear) */
    EXTI->INTFR = 1UL << pin_num;

    unsigned irqn = _irq_num(pin_num);
    pfic_set_handler(irqn, _gpio_isr);
    pfic_enable_interrupt(irqn, CPU_DEFAULT_IRQ_PRIO);

    EXTI->INTENR |= 1UL << pin_num;

    return 0;
}

void gpio_irq_enable(gpio_t pin)
{
    EXTI->INTENR |= 1UL << _pin_num(pin);
}

void gpio_irq_disable(gpio_t pin)
{
    EXTI->INTENR &= ~(1UL << _pin_num(pin));
}

static void _gpio_isr(unsigned irqn)
{
    (void)irqn;

    /* only handle lines which are unmasked */
    uint32_t pending_isr = EXTI->INTFR & EXTI->INTENR & GPIO_ISR_CHAN_MASK;

    /* clear by writing a 1 */
    EXTI->INTFR = pending_isr;

    uint8_t pin = 0;
    while (pending_isr) {
        pending_isr = bitarithm_test_and_clear(pending_isr, &pin);
        exti_ctx[pin].cb(exti_ctx[pin].arg);
    }
}
#endif /* MODULE_PERIPH_GPIO_IRQ */

/** @} */
