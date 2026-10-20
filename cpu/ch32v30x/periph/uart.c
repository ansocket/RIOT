/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup         cpu_ch32v30x
 * @{
 *
 * @file
 * @brief           CH32V30x UART peripheral implementation
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 * @author          ansocket
 */
#include <assert.h>
#include <stdint.h>

#include "cpu.h"
#include "periph/uart.h"
#include "periph_conf.h"
#include "periph_cpu.h"
#include "pfic.h"

#define RXENABLE            (USART_CTLR1_RE | USART_CTLR1_RXNEIE)

/**
 * @brief   Allocate memory to store the callback functions
 */
static uart_isr_ctx_t isr_ctx[UART_NUMOF];

static void _uart_isr(unsigned irq);

static inline USART_TypeDef *dev(uart_t uart)
{
    return uart_config[uart].dev;
}

int uart_init(uart_t uart, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    assert(uart < UART_NUMOF);

    /* save ISR context */
    isr_ctx[uart].rx_cb = rx_cb;
    isr_ctx[uart].arg = arg;

    periph_clk_en(uart_config[uart].bus, uart_config[uart].rcc_mask);

    /* reset UART configuration -> defaults to 8N1 mode */
    dev(uart)->CTLR1 = 0;
    dev(uart)->CTLR2 = 0;
    dev(uart)->CTLR3 = 0;

    /* 16x oversampling: BRR holds the divider in 1/16 steps */
    uint32_t clk = periph_apb_clk(uart_config[uart].bus);
    dev(uart)->BRR = (clk + baudrate / 2) / baudrate;

    /* Attach pins to the configured UART, otherwise the signal level
     * flickers during initialization resulting in garbage being sent */
    gpio_init_af(uart_config[uart].tx_pin, GPIO_AF_OUT_PP);
    if (rx_cb) {
        gpio_init_af(uart_config[uart].rx_pin, GPIO_AF_IN_PU);
    }

    /* enable RX interrupt if applicable */
    if (rx_cb) {
        pfic_set_handler(uart_config[uart].irqn, _uart_isr);
        pfic_enable_interrupt(uart_config[uart].irqn, CPU_DEFAULT_IRQ_PRIO);
        dev(uart)->CTLR1 = USART_CTLR1_UE | USART_CTLR1_TE | RXENABLE;
    }
    else {
        dev(uart)->CTLR1 = USART_CTLR1_UE | USART_CTLR1_TE;
    }

    return UART_OK;
}

void uart_write(uart_t uart, const uint8_t *data, size_t len)
{
    assert(uart < UART_NUMOF);

    for (size_t i = 0; i < len; i++) {
        while (!(dev(uart)->STATR & USART_STATR_TXE)) {}
        dev(uart)->DATAR = data[i];
    }
    /* make sure the function is synchronous by waiting for the transfer to
     * finish */
    while (!(dev(uart)->STATR & USART_STATR_TC)) {}
}

void uart_poweron(uart_t uart)
{
    assert(uart < UART_NUMOF);

    periph_clk_en(uart_config[uart].bus, uart_config[uart].rcc_mask);
    dev(uart)->CTLR1 |= USART_CTLR1_UE;
}

void uart_poweroff(uart_t uart)
{
    assert(uart < UART_NUMOF);

    dev(uart)->CTLR1 &= ~USART_CTLR1_UE;
    periph_clk_dis(uart_config[uart].bus, uart_config[uart].rcc_mask);
}

static void _uart_isr(unsigned irq)
{
    for (uart_t uart = 0; uart < UART_NUMOF; uart++) {
        if (uart_config[uart].irqn != irq) {
            continue;
        }

        uint32_t status = dev(uart)->STATR;

        /* reading DATAR clears both RXNE and ORE */
        if (status & (USART_STATR_RXNE | USART_STATR_ORE)) {
            uint8_t data = dev(uart)->DATAR;
            if (status & USART_STATR_RXNE) {
                isr_ctx[uart].rx_cb(isr_ctx[uart].arg, data);
            }
        }
        return;
    }
}
