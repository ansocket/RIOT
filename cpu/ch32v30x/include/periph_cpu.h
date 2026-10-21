/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-FileCopyrightText: 2023 Gunar Schorcht <gunar@schorcht.net>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup         cpu_gd32v
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 * @author          Gunar Schorcht <gunar@schorcht.net>
 */

#include <inttypes.h>

#include "cpu.h"
#include "ch32v30x.h"
#include "pfic.h"
#include "kernel_defines.h"
#include "macros/units.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   timer_set() is provided by the SysTick core timer driver
 */
#define PERIPH_TIMER_PROVIDES_SET

/**
 * @brief   Peripheral buses
 */
typedef enum {
    APB1,   /**< Advanced Peripheral Bus 1 */
    APB2,   /**< Advanced Peripheral Bus 2 */
} bus_t;

/**
 * @name    Override the default GPIO type
 * @{
 */
#define HAVE_GPIO_T
typedef uint32_t gpio_t;
/** @} */

/**
 * @brief   Available GPIO ports
 */
enum {
    PORT_A = 0,     /**< port A */
    PORT_B = 1,     /**< port B */
    PORT_C = 2,     /**< port C */
    PORT_D = 3,     /**< port D */
    PORT_E = 4,     /**< port E */
};

/**
 * @brief   Define a CPU specific GPIO pin generator macro
 */
#define GPIO_PIN(x, y)          ((gpio_t)(((x) << 4) | (y)))

#ifndef DOXYGEN
/**
 * @brief   Override GPIO mode options
 *
 * Bits 0-3 hold the CNF/MODE nibble of the CFGxR registers, bit 4 is the
 * OUTDR value which selects pull-up (1) or pull-down (0) for inputs.
 * @{
 */
#define HAVE_GPIO_MODE_T
typedef enum {
    GPIO_IN     = 0x04,     /**< input w/o pull R */
    GPIO_IN_PD  = 0x08,     /**< input with pull-down */
    GPIO_IN_PU  = 0x18,     /**< input with pull-up */
    GPIO_OUT    = 0x03,     /**< push-pull output, 50 MHz */
    GPIO_OD     = 0x07,     /**< open-drain output w/o pull R, 50 MHz */
    GPIO_OD_PU  = 0xff,     /**< not supported by HW */
} gpio_mode_t;
/** @} */

/**
 * @brief   Override flank configuration values
 * @{
 */
#define HAVE_GPIO_FLANK_T
typedef enum {
    GPIO_RISING  = 1,       /**< emit interrupt on rising flank */
    GPIO_FALLING = 2,       /**< emit interrupt on falling flank */
    GPIO_BOTH    = 3,       /**< emit interrupt on both flanks */
} gpio_flank_t;
/** @} */
#endif /* ndef DOXYGEN */

/**
 * @brief   Pin configurations for alternate function use
 *
 * The values are the CNF/MODE nibble of the CFGxR registers.
 */
typedef enum {
    GPIO_AF_IN_FLOAT = 0x4, /**< input, floating */
    GPIO_AF_IN_PU    = 0x8, /**< input with pull-up */
    GPIO_AF_OUT_PP   = 0xb, /**< alternate function output - push-pull */
    GPIO_AF_OUT_OD   = 0xf, /**< alternate function output - open-drain */
} gpio_af_t;

/**
 * @brief   Configure a pin for alternate function use
 *
 * @param[in] pin       pin to configure
 * @param[in] af        pin configuration
 */
void gpio_init_af(gpio_t pin, gpio_af_t af);

/**
 * @brief   Configure a pin as analog input (e.g. for the ADC)
 *
 * @param[in] pin       pin to configure
 */
void gpio_init_analog(gpio_t pin);

/**
 * @brief   Enable the clock of a peripheral
 *
 * @param[in] bus       bus the peripheral is connected to
 * @param[in] mask      bit in the clock enable register
 */
void periph_clk_en(bus_t bus, uint32_t mask);

/**
 * @brief   Disable the clock of a peripheral
 *
 * @param[in] bus       bus the peripheral is connected to
 * @param[in] mask      bit in the clock enable register
 */
void periph_clk_dis(bus_t bus, uint32_t mask);

/**
 * @brief   Get the clock frequency of an APB bus
 *
 * @param[in] bus       target APBx bus
 *
 * @return  bus clock frequency in Hz
 */
uint32_t periph_apb_clk(bus_t bus);

/**
 * @brief   Structure for UART configuration data
 */
typedef struct {
    USART_TypeDef *dev;     /**< UART device base register address */
    uint32_t rcc_mask;      /**< bit in clock enable register */
    gpio_t rx_pin;          /**< RX pin */
    gpio_t tx_pin;          /**< TX pin */
    bus_t bus;              /**< APB bus */
    uint8_t irqn;           /**< PFIC interrupt number */
} uart_conf_t;

#ifdef __cplusplus
}
#endif

/** @} */
