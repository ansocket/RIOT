/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_riscv_common
 * @{
 *
 * @file
 * @brief       Low-level timer implementation based on the WCH QingKe SysTick
 *
 * QingKe cores have no CLINT. Their core timer is a 64 bit SysTick counter
 * with a 64 bit compare register, its interrupt is routed through the PFIC.
 *
 * The timer runs either at HCLK or at HCLK / 8, the requested frequency
 * selects the clock source. `CLOCK_AHB` (or `CLOCK_CORECLOCK`) must match the
 * actual HCLK.
 *
 * @author      ansocket
 * @}
 */

#include <stdint.h>

#include "cpu.h"
#include "irq.h"
#include "periph_conf.h"
#include "periph/timer.h"
#include "pfic.h"

#ifndef QINGKE_SYSTICK_BASE_ADDR
/**
 * @brief   SysTick base address
 */
#  define QINGKE_SYSTICK_BASE_ADDR  (0xE000F000UL)
#endif

#ifndef CLOCK_AHB
#  define CLOCK_AHB                 CLOCK_CORECLOCK
#endif

/**
 * @brief   PFIC interrupt number of the SysTick
 */
#define SYSTICK_IRQN                (12)

/**
 * @name    SysTick CTLR bits
 * @{
 */
#define SYSTICK_CTLR_STE            (1UL << 0)  /**< Counter enable */
#define SYSTICK_CTLR_STIE           (1UL << 1)  /**< Interrupt enable */
#define SYSTICK_CTLR_STCLK          (1UL << 2)  /**< 1: HCLK, 0: HCLK / 8 */
/** @} */

/**
 * @brief   SysTick register map
 *
 * The 64 bit registers are accessed as 32 bit halves to control the access
 * order.
 */
typedef struct {
    volatile uint32_t CTLR;     /**< Control */
    volatile uint32_t SR;       /**< Status, bit 0 is the compare flag */
    volatile uint32_t CNTL;     /**< Counter, low word */
    volatile uint32_t CNTH;     /**< Counter, high word */
    volatile uint32_t CMPL;     /**< Compare, low word */
    volatile uint32_t CMPH;     /**< Compare, high word */
} systick_t;

#define SYSTICK                     ((systick_t *)QINGKE_SYSTICK_BASE_ADDR)

/**
 * @brief   Save reference to the timer callback
 */
static timer_cb_t isr_cb;

/**
 * @brief   Save argument for the callback
 */
static void *isr_arg;

void timer_isr(void);

static void _systick_isr(unsigned irq)
{
    (void)irq;
    timer_isr();
}

static uint64_t _read_cnt(void)
{
    uint32_t hi, lo;

    /* Re-read if the low word overflowed between the two reads */
    do {
        hi = SYSTICK->CNTH;
        lo = SYSTICK->CNTL;
    } while (hi != SYSTICK->CNTH);

    return ((uint64_t)hi << 32) | lo;
}

static void _set_cmp(uint64_t then)
{
    unsigned state = irq_disable();

    SYSTICK->CTLR &= ~SYSTICK_CTLR_STIE;

    /* Avoid a spurious match while the halves are updated */
    SYSTICK->CMPH = UINT32_MAX;
    SYSTICK->CMPL = (uint32_t)then;
    SYSTICK->CMPH = (uint32_t)(then >> 32);

    SYSTICK->SR = 0;
    SYSTICK->CTLR |= SYSTICK_CTLR_STIE;

    /* The target may have passed while programming the compare register */
    if (_read_cnt() >= then) {
        pfic_set_pending(SYSTICK_IRQN);
    }

    irq_restore(state);
}

int timer_init(tim_t dev, uint32_t freq, timer_cb_t cb, void *arg)
{
    uint32_t clk;

    if (dev != 0) {
        return -1;
    }

    if (freq == CLOCK_AHB) {
        clk = SYSTICK_CTLR_STCLK;
    }
    else if (freq == CLOCK_AHB / 8) {
        clk = 0;
    }
    else {
        return -1;
    }

    /* Save timer callback and arg */
    isr_cb = cb;
    isr_arg = arg;

    /* Stop the counter and reset it, counting up and free running */
    SYSTICK->CTLR = 0;
    SYSTICK->SR = 0;
    SYSTICK->CMPL = UINT32_MAX;
    SYSTICK->CMPH = UINT32_MAX;
    SYSTICK->CNTL = 0;
    SYSTICK->CNTH = 0;

    pfic_set_handler(SYSTICK_IRQN, _systick_isr);
    pfic_enable_interrupt(SYSTICK_IRQN, CPU_DEFAULT_IRQ_PRIO);

    SYSTICK->CTLR = SYSTICK_CTLR_STE | clk;

    return 0;
}

int timer_set(tim_t dev, int channel, unsigned int timeout)
{
    if (dev != 0 || channel != 0) {
        return -1;
    }

    _set_cmp(_read_cnt() + timeout);
    return 0;
}

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{
    if (dev != 0 || channel != 0) {
        return -1;
    }

    uint64_t now = _read_cnt();
    uint64_t then = (now & 0xFFFFFFFF00000000) + (uint64_t)value;

    /* Handle 32 bit rollover */
    _set_cmp((then > now) ? then : then + 0x100000000);
    return 0;
}

int timer_clear(tim_t dev, int channel)
{
    if (dev != 0 || channel != 0) {
        return -1;
    }

    unsigned state = irq_disable();
    SYSTICK->CTLR &= ~SYSTICK_CTLR_STIE;
    irq_restore(state);
    return 0;
}

unsigned int timer_read(tim_t dev)
{
    if (dev != 0) {
        return 0;
    }

    return SYSTICK->CNTL;
}

void timer_start(tim_t dev)
{
    if (dev != 0) {
        return;
    }

    unsigned state = irq_disable();
    SYSTICK->CTLR |= SYSTICK_CTLR_STE;
    irq_restore(state);
}

void timer_stop(tim_t dev)
{
    if (dev != 0) {
        return;
    }

    unsigned state = irq_disable();
    SYSTICK->CTLR &= ~SYSTICK_CTLR_STE;
    irq_restore(state);
}

void timer_isr(void)
{
    /* One shot: disarm before clearing the flag */
    SYSTICK->CTLR &= ~SYSTICK_CTLR_STIE;
    SYSTICK->SR = 0;

    if (isr_cb) {
        isr_cb(isr_arg, 0);
    }
}
