/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_ch32v30x
 * @ingroup     drivers_periph_wdt
 * @{
 *
 * @file
 * @brief       Independent watchdog driver for CH32V30x
 *
 * The watchdog runs from LSI independently of the CPU/APB clocks. Only
 * normal reset mode is supported. Once started, it runs until reset.
 */

#include <assert.h>

#include "periph/wdt.h"

#define IWDG_KEY_UNLOCK         (0x5555U)
#define IWDG_KEY_LOCK           (0x0000U)
#define IWDG_KEY_RELOAD         (0xaaaaU)
#define IWDG_KEY_START          (0xccccU)
#define IWDG_MAX_PRESCALER      (6U)
#define IWDG_COUNTER_STEPS      (IWDG_RLDR_RL_Msk + 1U)

void wdt_setup_reboot(uint32_t min_time, uint32_t max_time)
{
    /* IWDG does not provide a watchdog window or an early warning IRQ. */
    assert(min_time == 0);
    assert((max_time >= NWDT_TIME_LOWER_LIMIT) &&
           (max_time <= NWDT_TIME_UPPER_LIMIT));
    (void)min_time;

    /* Timeout = (reload + 1) * (4 << prescaler) / f_LSI.
     * Round up to a whole tick, then choose the smallest fitting divider.
     * The public timeout limits keep this arithmetic within uint32_t. */
    uint32_t cycles = max_time * WDT_CLOCK_FREQUENCY;
    unsigned prescaler = 0;
    uint32_t divisor = 4U * 1000U;
    uint32_t ticks = (cycles + divisor - 1U) / divisor;

    while ((ticks > IWDG_COUNTER_STEPS) && (prescaler < IWDG_MAX_PRESCALER)) {
        prescaler++;
        divisor <<= 1;
        ticks = (cycles + divisor - 1U) / divisor;
    }
    assert((ticks >= 1U) && (ticks <= IWDG_COUNTER_STEPS));

    /* Supply the independent clock before waiting for register updates.
     * This enables LSI without starting the watchdog counter. */
    RCC->RSTSCKR |= RCC_RSTSCKR_LSION;
    while (!(RCC->RSTSCKR & RCC_RSTSCKR_LSIRDY)) {}

    while (IWDG->STATR & (IWDG_STATR_PVU | IWDG_STATR_RVU)) {}
    IWDG->CTLR = IWDG_KEY_UNLOCK;
    IWDG->PSCR = prescaler;
    while (IWDG->STATR & IWDG_STATR_PVU) {}
    IWDG->RLDR = ticks - 1U;
    while (IWDG->STATR & IWDG_STATR_RVU) {}
    IWDG->CTLR = IWDG_KEY_LOCK;

    wdt_kick();
}

void wdt_start(void)
{
    wdt_kick();
    IWDG->CTLR = IWDG_KEY_START;
}

void wdt_kick(void)
{
    IWDG->CTLR = IWDG_KEY_RELOAD;
}

/** @} */
