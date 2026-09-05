/*
 * SPDX-FileCopyrightText: 2021 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     boards_lora-e5-dev
 * @{
 *
 * @file
 * @brief       Board specific implementations for the LoRa-E5 Development Board - STM32WLE5JC board
 *
 * @author      Francisco Molina <francois-xavier.molina@inria/fr>
 *
 * @}
 */

#include <assert.h>
#include "kernel_defines.h"
#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"

#if IS_USED(MODULE_SX126X_STM32WL)
#include "sx126x.h"
#endif

void board_init(void)
{
    if (IS_ACTIVE(CONFIG_E77_900MBL_01_ENABLE_3P3V)) {
        gpio_init(E77_900MBL_01_3P3V_ENABLE_PIN, GPIO_OUT);
        gpio_set(E77_900MBL_01_3P3V_ENABLE_PIN);
    }

    if (IS_ACTIVE(CONFIG_E77_900MBL_01_ENABLE_5V)) {
        gpio_init(E77_900MBL_01_5V_ENABLE_PIN, GPIO_OUT);
        gpio_set(E77_900MBL_01_5V_ENABLE_PIN);
    }

    if (IS_USED(MODULE_SX126X_STM32WL)) {
        /* Initialize the GPIO control for RF 3-port switch (SP3T) */
        gpio_init(FE_CTRL1, GPIO_OUT);
        gpio_init(FE_CTRL2, GPIO_OUT);
    }
}

#if IS_USED(MODULE_SX126X_STM32WL)
/**
 * @brief Callback to set RF switch mode
 *
 * This function sets the GPIO's wired to the SP3T RF Switch. LoRa-E5-dev
 * supports two modes of operation.
 */
void e77_900mbl_01_sx126x_set_rf_mode(sx126x_t *dev, sx126x_rf_mode_t rf_mode)
{
    (void) dev;
    switch (rf_mode) {
    case SX126X_RF_MODE_RX:
        gpio_set(FE_CTRL1);
        gpio_clear(FE_CTRL2);
        break;
    case SX126X_RF_MODE_TX_HPA:
        gpio_clear(FE_CTRL1);
        gpio_set(FE_CTRL2);
        break;
    default:
        /* SX126X_RF_MODE_TX_LPA is not supported */
        assert(0);
        break;
    }
}
#endif
