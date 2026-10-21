/*
 * SPDX-FileCopyrightText: 2024 Marian Buschsieweke
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     boards_gd32vf103c_start
 * @{
 *
 * @file
 * @brief       Board specific definitions for the GD32VF103C-START board
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

#include "board_common.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    System timer frequency
 * @{
 */
#define CONFIG_ZTIMER_USEC_BASE_FREQ (CLOCK_AHB / 8) /**< ztimer base = SysTick */
/** @} */

/**
 * @name    LED (on-board) configuration
 *
 * LED0 is labeled LED1 on the board, LED1 is labeled LED2.
 * Both LEDs are active low.
 * @{
 */
#define LED0_PIN            GPIO_PIN(PORT_A, 15)
#define LED0_MASK           (1 << 15)
#define LED0_IS_INVERTED    1
#define LED0_ON             (GPIOA->BCR = LED0_MASK)
#define LED0_OFF            (GPIOA->BSHR = LED0_MASK)
#define LED0_TOGGLE         (GPIOA->OUTDR ^= LED0_MASK)

#define LED1_PIN            GPIO_PIN(PORT_B, 4)
#define LED1_MASK           (1 << 4)
#define LED1_IS_INVERTED    1
#define LED1_ON             (GPIOB->BCR = LED1_MASK)
#define LED1_OFF            (GPIOB->BSHR = LED1_MASK)
#define LED1_TOGGLE         (GPIOB->OUTDR ^= LED1_MASK)
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
