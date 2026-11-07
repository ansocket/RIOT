/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-FileCopyrightText: 2023 Gunar Schorcht <gunar@schorcht.net>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     boards_ch32v307_board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the YD-CH32V307VCT6 board
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

#include "macros/units.h"

#ifndef CONFIG_BOARD_HAS_HXTAL
#define CONFIG_BOARD_HAS_HXTAL  1   /**< The board provides a high frequency oscillator. */
#endif

#ifndef CONFIG_BOARD_HAS_LXTAL
#define CONFIG_BOARD_HAS_LXTAL  1   /**< The board provides a low frequency oscillator. */
#endif

#ifndef CONFIG_CLOCK_HXTAL
#define CONFIG_CLOCK_HXTAL      MHZ(8)      /**< HXTAL frequency */
#endif


#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The clock tree is not configured yet, the MCU runs from the HSI with the
 * reset prescalers (all 1) */
#define CLOCK_CORECLOCK         MHZ(8)      /**< CPU clock frequency in Hz */

#define CLOCK_AHB               CLOCK_CORECLOCK /**< Equal to the CPU clock */
#define CLOCK_APB1              CLOCK_AHB       /**< Equal to the AHB clock */
#define CLOCK_APB2              CLOCK_AHB       /**< Equal to the AHB clock */

/**
 * @name    RTT/RTC configuration
 * @{
 */
#if CONFIG_BOARD_HAS_LXTAL
#define RTT_CLOCK_FREQUENCY (32768U)        /**< Low frequency XTAL is used as clock source */
#else
#define RTT_CLOCK_FREQUENCY (40000U)        /**< IRC40K is used as clock source */
#endif

#define RTT_MAX_FREQUENCY   (RTT_CLOCK_FREQUENCY)   /* maximum RTT frequency in Hz */

#ifndef RTT_FREQUENCY
#define RTT_FREQUENCY       (RTT_MAX_FREQUENCY)     /* RTT frequency in Hz */
#endif
/** @} */

/**
 * @name    Timer configuration
 *
 * One timer via the QingKe SysTick (core timer).
 * @{
 */
#define TIMER_NUMOF             (1)
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev = USART1,
        .rcc_mask = RCC_APB2PCENR_USART1EN,
        .rx_pin = GPIO_PIN(PORT_A, 10),
        .tx_pin = GPIO_PIN(PORT_A, 9),
        .bus = APB2,
        .irqn = USART1_IRQn,
    },
};

#define UART_NUMOF              ARRAY_SIZE(uart_config)
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
