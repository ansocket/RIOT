/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_ch32v30x
 * @{
 * @file
 * @brief       CH32V30x SDIO host driver for SD memory cards
 *
 * Transfers use the FIFO with hardware flow control and software deadlines.
 * The clock is limited to 4 MHz for this polling implementation.
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include "bitarithm.h"
#include "log.h"
#include "periph/gpio.h"
#include "sdmmc/sdmmc.h"
#include "ztimer.h"

#define SDIO_CMD_FLAGS          (SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT | \
                                 SDIO_STA_CMDREND | SDIO_STA_CMDSENT)
#define SDIO_DATA_ERRORS        (SDIO_STA_DCRCFAIL | SDIO_STA_DTIMEOUT | \
                                 SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR | \
                                 SDIO_STA_STBITERR)
#define SDIO_DATA_FLAGS         (SDIO_DATA_ERRORS | SDIO_STA_DATAEND | SDIO_STA_DBCKEND)
#define SDIO_CMD_TIMEOUT_US     (100000U)
#define SDIO_DATA_TIMEOUT_US    (500000U)
#define SDIO_MAX_CLOCK          MHZ(4)
#define SDIO_MAX_DATA_LEN       (0x1ffffffUL)

#if CLOCK_AHB > (KHZ(400) * (SDIO_CLKCR_CLKDIV_Msk + 2))
#error "CH32V30x SDIO requires HCLK <= 102.8 MHz for card identification"
#endif

static const sdmmc_driver_t _driver;
static sdmmc_dev_t _dev = { .driver = &_driver };
static uint32_t _clock;

static_assert(SDMMC_CONFIG_NUMOF == 1, "CH32V30x has one SDIO controller");
static_assert(ARRAY_SIZE(sdmmc_config) == SDMMC_CONFIG_NUMOF,
              "SDMMC_CONFIG_NUMOF differs from sdmmc_config size");

XFA_CONST(sdmmc_dev_t * const, sdmmc_devs, 0) _ch32v30x_sdmmc = &_dev;

static bool _present(void)
{
    const sdmmc_conf_t *conf = &sdmmc_config[0];

    return !gpio_is_valid(conf->cd) ||
           (!!gpio_read(conf->cd) == !!conf->cd_active);
}

static void _card_detect(void *arg)
{
    sdmmc_dev_t *dev = arg;

    dev->present = _present();
    dev->init_done = false;
    if (dev->event_cb) {
        dev->event_cb(dev, dev->present ? SDMMC_EVENT_CARD_INSERTED :
                                        SDMMC_EVENT_CARD_REMOVED);
    }
}

static void _stop_data(void)
{
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;

    sdio->DCTRL = 0;
    /* Drain only the hardware FIFO capacity, even if the peripheral is faulty. */
    for (unsigned i = 0; i < 32 && (sdio->STA & SDIO_STA_RXDAVL); i++) {
        (void)sdio->FIFO;
    }
    sdio->ICR = SDIO_DATA_FLAGS;
}

static void _clock_update_delay(void)
{
    /* periph_init() runs before ztimer initialization. The manual requires
     * at least seven HCLK cycles between clock register updates. */
    for (unsigned i = 0; i < 8; i++) {
        __asm__ volatile ("nop");
    }
}

static int _set_clock_rate(sdmmc_dev_t *dev, sdmmc_clock_rate_t rate)
{
    (void)dev;

    if (rate != SDMMC_CLK_400K && rate != SDMMC_CLK_25M) {
        return -ENOTSUP;
    }

    uint32_t target = (rate == SDMMC_CLK_400K) ? KHZ(400) : SDIO_MAX_CLOCK;
    /* Round up so the SD clock never exceeds the requested frequency. */
    uint32_t divisor = (CLOCK_AHB + target - 1) / target;
    if (divisor < 2) {
        divisor = 2;
    }
    if (divisor > SDIO_CLKCR_CLKDIV_Msk + 2) {
        return -ENOTSUP;
    }
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;
    sdio->CLKCR = (sdio->CLKCR & ~(SDIO_CLKCR_CLKDIV_Msk | SDIO_CLKCR_BYPASS)) |
                  (divisor - 2);
    _clock = CLOCK_AHB / divisor;
    _clock_update_delay();
    return 0;
}

static int _set_bus_width(sdmmc_dev_t *dev, sdmmc_bus_width_t width)
{
    if (width != SDMMC_BUS_WIDTH_1BIT &&
        (width != SDMMC_BUS_WIDTH_4BIT || dev->bus_width != SDMMC_BUS_WIDTH_4BIT)) {
        return -ENOTSUP;
    }
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;
    sdio->CLKCR = (sdio->CLKCR & ~SDIO_CLKCR_WIDBUS_Msk) |
                  ((width == SDMMC_BUS_WIDTH_4BIT) ? SDIO_CLKCR_WIDBUS_4BIT : 0);
    _clock_update_delay();
    return 0;
}

static int _enable_clock(sdmmc_dev_t *dev, bool enable)
{
    (void)dev;
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;

    if (enable) {
        sdio->CLKCR |= SDIO_CLKCR_CLKEN;
    }
    else {
        /* Also clean up a prepared read if its command failed. */
        _stop_data();
        sdio->CLKCR &= ~SDIO_CLKCR_CLKEN;
    }
    _clock_update_delay();
    return 0;
}

static void _init(sdmmc_dev_t *dev)
{
    const sdmmc_conf_t *conf = &sdmmc_config[0];

    assert(dev == &_dev);
    assert(conf->dev == SDIO);
    /* SDIO has fixed pins and no alternate pin mapping. */
    assert(conf->clk == GPIO_PIN(PORT_C, 12));
    assert(conf->cmd == GPIO_PIN(PORT_D, 2));
    assert(conf->dat0 == GPIO_PIN(PORT_C, 8));
    bool wide = gpio_is_valid(conf->dat1) || gpio_is_valid(conf->dat2) ||
                gpio_is_valid(conf->dat3);
    assert(!wide || (conf->dat1 == GPIO_PIN(PORT_C, 9) &&
                     conf->dat2 == GPIO_PIN(PORT_C, 10) &&
                     conf->dat3 == GPIO_PIN(PORT_C, 11)));

    gpio_init_af(conf->clk, GPIO_AF_OUT_PP);
    gpio_init_af(conf->cmd, GPIO_AF_OUT_PP);
    gpio_init_af(conf->dat0, GPIO_AF_OUT_PP);
    if (wide) {
        gpio_init_af(conf->dat1, GPIO_AF_OUT_PP);
        gpio_init_af(conf->dat2, GPIO_AF_OUT_PP);
        gpio_init_af(conf->dat3, GPIO_AF_OUT_PP);
    }

    RCC->AHBPCENR |= RCC_AHBPCENR_SDIOEN;
    conf->dev->MASK = 0;
    conf->dev->CMD = 0;
    conf->dev->CLKCR = 0;
    conf->dev->POWER = 0;
    _stop_data();
    conf->dev->DLEN = 0;
    conf->dev->DTIMER = 0;
    conf->dev->DCTRL2 = 0;
    conf->dev->ICR = SDIO_CMD_FLAGS | SDIO_DATA_FLAGS;
    conf->dev->CLKCR = SDIO_CLKCR_HWFC_EN;
    int res = _set_clock_rate(dev, SDMMC_CLK_400K);
    assert(res == 0);
    (void)res;
    conf->dev->POWER = SDIO_POWER_ON;
    _enable_clock(dev, true);

    dev->bus_width = wide ? SDMMC_BUS_WIDTH_4BIT : SDMMC_BUS_WIDTH_1BIT;
    dev->spi_mode = false;
    dev->s18v_support = false;
    dev->init_done = false;
    if (gpio_is_valid(conf->cd)) {
        res = gpio_init_int(conf->cd, conf->cd_mode, GPIO_BOTH, _card_detect, dev);
        assert(res == 0);
        (void)res;
    }
    dev->present = _present();
}

static int _send_cmd(sdmmc_dev_t *dev, sdmmc_cmd_t cmd_idx, uint32_t arg,
                     sdmmc_resp_t resp_type, uint32_t *resp)
{
    assert(!(cmd_idx & SDMMC_ACMD_PREFIX));
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;

    if (!_present()) {
        _stop_data();
        return -ENODEV;
    }
    uint32_t cmd = cmd_idx & 0x3f;
    uint32_t complete = SDIO_STA_CMDSENT;
    if (resp_type != SDMMC_NO_R) {
        cmd |= (resp_type == SDMMC_R2) ? SDIO_CMD_WAITRESP_LONG : SDIO_CMD_WAITRESP_SHORT;
        complete = SDIO_STA_CMDREND | SDIO_STA_CCRCFAIL;
    }

    sdio->CMD &= ~SDIO_CMD_CPSMEN;
    /* ICR flags are cleared by writing ones, before starting the command. */
    sdio->ICR = SDIO_CMD_FLAGS;
    sdio->ARG = arg;
    sdio->CMD = cmd | SDIO_CMD_CPSMEN;
    uint32_t start = ztimer_now(ZTIMER_USEC);
    uint32_t status;
    int res = 0;
    while (1) {
        status = sdio->STA;
        if ((status & SDIO_STA_CTIMEOUT) ||
            (uint32_t)(ztimer_now(ZTIMER_USEC) - start) >= SDIO_CMD_TIMEOUT_US) {
            res = -ETIMEDOUT;
            break;
        }
        if (status & complete) {
            break;
        }
    }
    if (!res && (status & SDIO_STA_CCRCFAIL) && (resp_type & SDMMC_RESP_CRC)) {
        res = -EBADMSG;
    }
    if (!res && resp_type != SDMMC_NO_R) {
        uint32_t response_cmd = sdio->RESPCMD;
        /* Follow the WCH R7 sequence: use CRC and the CMD8 check pattern
         * (validated by the SDMMC layer), without relying on RESPCMD.
         * R2, R3 and R4 also do not carry an echoed command index. */
        bool check_index = resp_type == SDMMC_R1 || resp_type == SDMMC_R1B ||
                           resp_type == SDMMC_R5 || resp_type == SDMMC_R6;
        if (check_index && (response_cmd & 0x3f) != (cmd_idx & 0x3f)) {
            LOG_ERROR("[ch32v30x sdmmc] CMD%u response index mismatch: "
                      "RESPCMD=%08" PRIx32 " STA=%08" PRIx32 " RESP1=%08" PRIx32 "\n",
                      (unsigned)cmd_idx, response_cmd, status, sdio->RESP[0]);
            res = -EIO;
        }
        else {
            if (resp_type == SDMMC_R1 || resp_type == SDMMC_R1B) {
                dev->status = sdio->RESP[0];
            }
            if (resp) {
                for (unsigned i = 0; i < ((resp_type == SDMMC_R2) ? 4U : 1U); i++) {
                    resp[i] = sdio->RESP[i];
                }
            }
        }
    }
    /* Read the response before changing CMD or clearing completion flags.
     * Preserve CMDINDEX and WAITRESP when disabling the command state machine. */
    sdio->CMD &= ~SDIO_CMD_CPSMEN;
    sdio->ICR = SDIO_CMD_FLAGS;
    if (res) {
        _stop_data();
    }
    return res;
}

static int _xfer_prepare(sdmmc_dev_t *dev, sdmmc_xfer_desc_t *xfer)
{
    (void)dev;
    uint32_t length = (uint32_t)xfer->block_size * xfer->block_num;

    if (xfer->type != SDMMC_BLOCK || !xfer->block_num || !xfer->block_size ||
        (xfer->block_size & (xfer->block_size - 1)) ||
        xfer->block_size > 16384 || length > SDIO_MAX_DATA_LEN) {
        return -EINVAL;
    }
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;
    _stop_data();
    sdio->DTIMER = xfer->write ? (_clock / 4) : (_clock / 10);
    sdio->DLEN = length;
    uint32_t ctrl = bitarithm_msb(xfer->block_size) << SDIO_DCTRL_DBLOCKSIZE_Pos;
    /* Arm reads before sending the command: a card may respond immediately. */
    sdio->DCTRL = ctrl | (xfer->write ? 0 : SDIO_DCTRL_DTDIR | SDIO_DCTRL_DTEN);
    return 0;
}

static int _xfer_execute(sdmmc_dev_t *dev, sdmmc_xfer_desc_t *xfer,
                         const void *data_wr, void *data_rd, uint16_t *done)
{
    (void)dev;
    assert((xfer->write && data_wr) || (!xfer->write && data_rd));
    SDIO_TypeDef *sdio = sdmmc_config[0].dev;
    const uint8_t *src = data_wr;
    uint8_t *dst = data_rd;
    uint32_t left = (uint32_t)xfer->block_size * xfer->block_num;
    uint32_t progress = ztimer_now(ZTIMER_USEC);
    int res = 0;

    if (done) {
        *done = 0;
    }
    if (xfer->write) {
        sdio->DCTRL |= SDIO_DCTRL_DTEN;
    }
    while (1) {
        uint32_t status = sdio->STA;
        if (status & SDIO_DATA_ERRORS) {
            if (status & SDIO_STA_DTIMEOUT) {
                res = -ETIMEDOUT;
            }
            else if (status & SDIO_STA_DCRCFAIL) {
                res = -EBADMSG;
            }
            else if (status & (SDIO_STA_RXOVERR | SDIO_STA_TXUNDERR)) {
                res = -ENOMEM;
            }
            else {
                res = -EIO;
            }
            break;
        }
        /* DATAEND alone does not guarantee that the final CRC was checked. */
        if (xfer->write && left && (status & SDIO_STA_DATAEND)) {
            res = -EIO;
            break;
        }
        if (left && ((!xfer->write && (status & SDIO_STA_RXDAVL)) ||
                     (xfer->write && (status & SDIO_STA_TXFIFOHE)))) {
            /* A half-empty TX FIFO always has room for eight words. */
            unsigned words = xfer->write ? 8 : 1;
            while (words-- && left) {
                uint32_t word = 0;
                size_t bytes = (left < sizeof(word)) ? left : sizeof(word);
                if (xfer->write) {
                    memcpy(&word, src, bytes);
                    sdio->FIFO = word;
                    src += bytes;
                }
                else {
                    word = sdio->FIFO;
                    memcpy(dst, &word, bytes);
                    dst += bytes;
                }
                left -= bytes;
            }
            progress = ztimer_now(ZTIMER_USEC);
            continue;
        }
        /* DBCKEND may still belong to an earlier block. Wait until the data
         * state machine has finished the final block, including its CRC. */
        if ((status & SDIO_STA_DATAEND) && (status & SDIO_STA_DBCKEND) &&
            !(status & (SDIO_STA_RXACT | SDIO_STA_TXACT))) {
            res = left ? -EIO : 0;
            break;
        }
        if (!_present()) {
            res = -ENODEV;
            break;
        }
        if ((uint32_t)(ztimer_now(ZTIMER_USEC) - progress) >= SDIO_DATA_TIMEOUT_US) {
            res = -ETIMEDOUT;
            break;
        }
    }
    _stop_data();
    if (!res && done) {
        *done = xfer->block_num;
    }
    return res;
}

static int _xfer_finish(sdmmc_dev_t *dev, sdmmc_xfer_desc_t *xfer)
{
    (void)dev;
    (void)xfer;
    _stop_data();
    return 0;
}

static const sdmmc_driver_t _driver = {
    .init = _init,
    .send_cmd = _send_cmd,
    .set_bus_width = _set_bus_width,
    .set_clock_rate = _set_clock_rate,
    .enable_clock = _enable_clock,
    .xfer_prepare = _xfer_prepare,
    .xfer_execute = _xfer_execute,
    .xfer_finish = _xfer_finish,
};
