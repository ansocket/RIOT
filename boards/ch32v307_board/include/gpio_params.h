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
 * @brief       Configuration of SAUL mapped GPIO pins
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    LED SAUL configuration
 */
static const  saul_gpio_params_t saul_gpio_params[] =
{
    {
        .name  = "LED1",
        .pin   = LED0_PIN,
        .mode  = GPIO_OUT,
        .flags = SAUL_GPIO_INVERTED | SAUL_GPIO_INIT_CLEAR,
    },
    {
        .name  = "LED2",
        .pin   = LED1_PIN,
        .mode  = GPIO_OUT,
        .flags = SAUL_GPIO_INVERTED | SAUL_GPIO_INIT_CLEAR,
    },
};

#ifdef __cplusplus
}
#endif

/** @} */
