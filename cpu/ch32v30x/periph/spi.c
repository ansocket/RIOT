/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_ch32v30x
 * @ingroup     drivers_periph_spi
 * @{
 *
 * @file
 * @brief       Polling SPI master driver for CH32V30x
 *
 * Supports 8-bit, MSB-first transfers, all four modes and software CS.
 * The APB clock is divided by powers of two from 2 to 256. Requests below
 * APB / 256 cannot be met and trigger an assertion in spi_acquire().
 */

#include <assert.h>

#include "mutex.h"
#include "periph/gpio.h"
#include "periph/spi.h"

static mutex_t _locks[SPI_NUMOF];

static inline SPI_TypeDef *_dev(spi_t bus)
{
    return spi_config[bus].dev;
}

static unsigned _prescaler(spi_t bus, spi_clk_t clk)
{
    static const uint32_t rates[] = {
        [SPI_CLK_100KHZ] = KHZ(100),
        [SPI_CLK_400KHZ] = KHZ(400),
        [SPI_CLK_1MHZ] = MHZ(1),
        [SPI_CLK_5MHZ] = MHZ(5),
        [SPI_CLK_10MHZ] = MHZ(10),
    };
    assert((unsigned)clk < ARRAY_SIZE(rates));

    uint32_t apb = periph_apb_clk(spi_config[bus].bus);
    unsigned br = 0;
    unsigned br_max = SPI_CTLR1_BR_Msk >> SPI_CTLR1_BR_Pos;

    while ((br < br_max) && (apb > (rates[clk] << (br + 1)))) {
        br++;
    }
    /* Never silently overclock a slave when the divider is exhausted. */
    assert(apb <= (rates[clk] << (br + 1)));
    return br;
}

void spi_init(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    mutex_init(&_locks[bus]);
    spi_init_pins(bus);

    periph_clk_en(spi_config[bus].bus, spi_config[bus].rcc_mask);
    _dev(bus)->CTLR1 = 0;
    _dev(bus)->CTLR2 = 0;
    _dev(bus)->I2SCFGR = 0;
    periph_clk_dis(spi_config[bus].bus, spi_config[bus].rcc_mask);
}

void spi_init_pins(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    gpio_init_af(spi_config[bus].sclk_pin, GPIO_AF_OUT_PP);
    if (gpio_is_valid(spi_config[bus].mosi_pin)) {
        gpio_init_af(spi_config[bus].mosi_pin, GPIO_AF_OUT_PP);
    }
    if (gpio_is_valid(spi_config[bus].miso_pin)) {
        gpio_init(spi_config[bus].miso_pin, GPIO_IN);
    }
}

void spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    assert(bus < SPI_NUMOF);
    assert((unsigned)mode <= SPI_MODE_3);
    (void)cs;

    mutex_lock(&_locks[bus]);
    unsigned br = _prescaler(bus, clk);

    periph_clk_en(spi_config[bus].bus, spi_config[bus].rcc_mask);
    /* Mode values match CPHA/CPOL. Configure with SPE clear, then enable.
     * SSM/SSI keep NSS high internally so no physical NSS pin is needed. */
    _dev(bus)->CTLR1 = SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI |
                        (br << SPI_CTLR1_BR_Pos) | mode;
    _dev(bus)->CTLR1 |= SPI_CTLR1_SPE;
}

void spi_release(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    while (!(_dev(bus)->STATR & SPI_STATR_TXE)) {}
    while (_dev(bus)->STATR & SPI_STATR_BSY) {}
    _dev(bus)->CTLR1 = 0;
    periph_clk_dis(spi_config[bus].bus, spi_config[bus].rcc_mask);
    mutex_unlock(&_locks[bus]);
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    assert(bus < SPI_NUMOF);
    assert(out || in);

    const uint8_t *tx = out;
    uint8_t *rx = in;
    SPI_TypeDef *dev = _dev(bus);
    volatile uint8_t *data = (volatile uint8_t *)&dev->DATAR;

    if (gpio_is_valid(cs)) {
        gpio_clear(cs);
    }

    for (size_t i = 0; i < len; i++) {
        while (!(dev->STATR & SPI_STATR_TXE)) {}
        *data = tx ? tx[i] : 0;
        while (!(dev->STATR & SPI_STATR_RXNE)) {}
        /* Always drain RX, including write-only transfers, to avoid overrun. */
        uint8_t byte = *data;
        if (rx) {
            rx[i] = byte;
        }
    }

    /* RXNE can be set before the last clock edge. Finish before releasing CS. */
    while (!(dev->STATR & SPI_STATR_TXE)) {}
    while (dev->STATR & SPI_STATR_BSY) {}

    if (!cont && gpio_is_valid(cs)) {
        gpio_set(cs);
    }
}

/** @} */
