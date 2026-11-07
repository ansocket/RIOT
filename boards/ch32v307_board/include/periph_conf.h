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
 * All eight capture/compare timers provide 16 bit counters and four channels.
 * TIM2 remains TIMER_DEV(0), the default ztimer clock. TIM6 and TIM7 are basic
 * timers without compare channels and cannot be used by this driver.
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        .dev = TIM2,
        .rcc_mask = RCC_APB1PCENR_TIM2EN,
        .bus = APB1,
        .irqn = TIM2_IRQn,
    },
    {
        .dev = TIM3,
        .rcc_mask = RCC_APB1PCENR_TIM3EN,
        .bus = APB1,
        .irqn = TIM3_IRQn,
    },
    {
        .dev = TIM4,
        .rcc_mask = RCC_APB1PCENR_TIM4EN,
        .bus = APB1,
        .irqn = TIM4_IRQn,
    },
    {
        .dev = TIM5,
        .rcc_mask = RCC_APB1PCENR_TIM5EN,
        .bus = APB1,
        .irqn = TIM5_IRQn,
    },
    {
        .dev = TIM1,
        .rcc_mask = RCC_APB2PCENR_TIM1EN,
        .bus = APB2,
        .irqn = TIM1_CC_IRQn,
    },
    {
        .dev = TIM8,
        .rcc_mask = RCC_APB2PCENR_TIM8EN,
        .bus = APB2,
        .irqn = TIM8_CC_IRQn,
    },
    {
        .dev = TIM9,
        .rcc_mask = RCC_APB2PCENR_TIM9EN,
        .bus = APB2,
        .irqn = TIM9_CC_IRQn,
    },
    {
        .dev = TIM10,
        .rcc_mask = RCC_APB2PCENR_TIM10EN,
        .bus = APB2,
        .irqn = TIM10_CC_IRQn,
    },
};

#define TIMER_NUMOF             ARRAY_SIZE(timer_config)
#define TIMER_0_MAX_VALUE       (0xffffU)
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
