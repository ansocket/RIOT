/*
 * SPDX-FileCopyrightText: Klim Evdokimov
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     boards_e77-900m
 * @{
 *
 * @file
 * @brief       board.h for e77-900m dev board
 *
 * @author      Klim Evdokimov <klimevdokimov@mail.ru>
 */

#include "kernel_defines.h"
#if IS_USED(MODULE_SX126X_STM32WL)
#include "sx126x.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Sub-GHz radio (LoRa) configuration
 * @{
 */
#define SX126X_PARAM_SPI                    (SPI_DEV(0))
#define SX126X_PARAM_TX_PA_MODE             SX126X_RF_MODE_TX_HPA
#if IS_USED(MODULE_SX126X_STM32WL)
extern void e77_900mbl_01_sx126x_set_rf_mode(sx126x_t *dev, sx126x_rf_mode_t rf_mode);
#define SX126X_PARAM_SET_RF_MODE_CB         e77_900mbl_01_sx126x_set_rf_mode
#define SX126X_PARAM_TYPE                   SX126X_TYPE_STM32WL
#endif
/** @} */

/**
 * @name    LED pin definitions and handlers
 * @{
 */
#define LED0_PORT           GPIO_PORT_B /**< GPIO port of LED 0 */
#define LED0_PORT_NUM       PORT_B
#define LED0_PIN_NUM        4
#define LED1_PORT           GPIO_PORT_B /**< GPIO port of LED 1 */
#define LED1_PORT_NUM       PORT_B
#define LED1_PIN_NUM        3
/** @} */

/**
 * @name    User button
 * @{
 */
#define BTN0_PIN            GPIO_PIN(PORT_A, 1)
#define BTN0_MODE           GPIO_IN_PU
#define BTN1_PIN            GPIO_PIN(PORT_A, 0)
#define BTN1_MODE           GPIO_IN_PU
/** @} */

/**
 * @name    RF 3-port switch (SP3T) control
 *
 * Refer Section 6.6.3 RF Overview in User Manual (UM2592)
 * @{
 */
#define FE_CTRL1            GPIO_PIN(PORT_A, 6)
#define FE_CTRL2            GPIO_PIN(PORT_A, 7)
/** @} */

/**
 * @brief    Enable 3.3V output
 */
#ifndef CONFIG_E77_900MBL_01_ENABLE_3P3V
#define CONFIG_E77_900MBL_01_ENABLE_3P3V      1
#endif
/**
 * @brief    Enable 5V output
 */
#ifndef CONFIG_E77_900MBL_01_ENABLE_5V
#define CONFIG_E77_900MBL_01_ENABLE_5V        1
#endif
/**
 * @brief    lora-e5-dev 3.3V gpio enable pin
 */
#define E77_900MBL_01_3P3V_ENABLE_PIN         GPIO_PIN(PORT_A, 9)
/**
 * @brief    lora-e5-dev 5V gpio enable pin
 */
#define E77_900MBL_01_5V_ENABLE_PIN           GPIO_PIN(PORT_B, 10)

#ifdef __cplusplus
}
#endif

#include "stm32_leds.h"

/** @} */
