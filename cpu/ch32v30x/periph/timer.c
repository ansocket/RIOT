/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_ch32v30x
 * @ingroup     drivers_periph_timer
 * @{
 *
 * @file
 * @brief       CH32V30x general purpose and advanced timer implementation
 *
 * @author      ansocket
 * @}
 */

#include <assert.h>
#include <stdint.h>

#include "irq.h"
#include "periph/timer.h"
#include "periph_conf.h"
#include "pfic.h"

#define TIMER_MAX_VALUE     (UINT16_MAX)

static timer_isr_ctx_t isr_ctx[TIMER_NUMOF];

static void _timer_isr(unsigned irq);

static inline TIM_TypeDef *dev(tim_t tim)
{
    return timer_config[tim].dev;
}

static uint32_t _timer_clk(tim_t tim)
{
    uint32_t clk = periph_apb_clk(timer_config[tim].bus);

    /* Timer clocks are doubled when the APB prescaler is not one. */
    return (clk == CLOCK_AHB) ? clk : 2 * clk;
}

int timer_init(tim_t tim, uint32_t freq, timer_cb_t cb, void *arg)
{
    if (tim >= TIMER_NUMOF || !freq) {
        return -1;
    }

    uint32_t clk = _timer_clk(tim);
    uint32_t divider = clk / freq;

    if (!divider || divider > UINT16_MAX + 1UL || clk % freq) {
        return -1;
    }

    unsigned state = irq_disable();
    pfic_disable_interrupt(timer_config[tim].irqn);
    periph_clk_en(timer_config[tim].bus, timer_config[tim].rcc_mask);

    dev(tim)->CTLR1 = 0;
    dev(tim)->CTLR2 = 0;
    dev(tim)->SMCFGR = 0;
    dev(tim)->DMAINTENR = 0;
    dev(tim)->CHCTLR1 = 0;
    dev(tim)->CHCTLR2 = 0;
    dev(tim)->CCER = 0;
    dev(tim)->PSC = divider - 1;
    dev(tim)->ATRLR = TIMER_MAX_VALUE;

    if (dev(tim) == TIM1 || dev(tim) == TIM8 ||
        dev(tim) == TIM9 || dev(tim) == TIM10) {
        /* Advanced timers must reload on every update event. */
        dev(tim)->RPTCR = 0;
    }

    for (unsigned channel = 0; channel < TIMER_CHANNEL_NUMOF; channel++) {
        dev(tim)->CHCVR[channel] = 0;
    }

    /* Apply the prescaler and reset the counter, then clear update flags. */
    dev(tim)->SWEVGR = TIM_SWEVGR_UG;
    dev(tim)->CNT = 0;
    dev(tim)->INTFR = 0;

    isr_ctx[tim].cb = cb;
    isr_ctx[tim].arg = arg;
    pfic_set_handler(timer_config[tim].irqn, _timer_isr);
    pfic_clear_pending(timer_config[tim].irqn);
    pfic_enable_interrupt(timer_config[tim].irqn, CPU_DEFAULT_IRQ_PRIO);
    dev(tim)->CTLR1 = TIM_CTLR1_CEN;
    irq_restore(state);

    return 0;
}

/* Called with interrupts disabled, with a timeout relative to a recent read. */
static void _set(tim_t tim, unsigned channel, unsigned value, unsigned timeout)
{
    uint32_t mask = TIM_DMAINTENR_CC1IE << channel;

    dev(tim)->DMAINTENR &= ~mask;
    dev(tim)->CHCVR[channel] = value;
    /* Flags are write-zero-to-clear: preserve pending matches on other channels. */
    dev(tim)->INTFR = ~(TIM_INTFR_CC1IF << channel);
    dev(tim)->DMAINTENR |= mask;

    unsigned remaining = (value - dev(tim)->CNT) & TIMER_MAX_VALUE;
    if (!remaining || remaining > timeout) {
        /* The deadline passed while programming the compare register. */
        dev(tim)->SWEVGR = TIM_SWEVGR_CC1G << channel;
    }
}

int timer_set(tim_t tim, int channel, unsigned int timeout)
{
    if (tim >= TIMER_NUMOF || (unsigned)channel >= TIMER_CHANNEL_NUMOF) {
        return -1;
    }

    unsigned state = irq_disable();
    timeout &= TIMER_MAX_VALUE;
    unsigned value = (dev(tim)->CNT + timeout) & TIMER_MAX_VALUE;
    _set(tim, channel, value, timeout);
    irq_restore(state);

    return 0;
}

int timer_set_absolute(tim_t tim, int channel, unsigned int value)
{
    if (tim >= TIMER_NUMOF || (unsigned)channel >= TIMER_CHANNEL_NUMOF) {
        return -1;
    }

    unsigned state = irq_disable();
    value &= TIMER_MAX_VALUE;
    unsigned timeout = (value - dev(tim)->CNT) & TIMER_MAX_VALUE;
    _set(tim, channel, value, timeout);
    irq_restore(state);

    return 0;
}

int timer_clear(tim_t tim, int channel)
{
    if (tim >= TIMER_NUMOF || (unsigned)channel >= TIMER_CHANNEL_NUMOF) {
        return -1;
    }

    unsigned state = irq_disable();
    dev(tim)->DMAINTENR &= ~(TIM_DMAINTENR_CC1IE << channel);
    dev(tim)->INTFR = ~(TIM_INTFR_CC1IF << channel);
    irq_restore(state);

    return 0;
}

unsigned int timer_read(tim_t tim)
{
    assert(tim < TIMER_NUMOF);
    return dev(tim)->CNT & TIMER_MAX_VALUE;
}

void timer_start(tim_t tim)
{
    assert(tim < TIMER_NUMOF);
    unsigned state = irq_disable();
    dev(tim)->CTLR1 |= TIM_CTLR1_CEN;
    irq_restore(state);
}

void timer_stop(tim_t tim)
{
    assert(tim < TIMER_NUMOF);
    unsigned state = irq_disable();
    dev(tim)->CTLR1 &= ~TIM_CTLR1_CEN;
    irq_restore(state);
}

static void _timer_isr(unsigned irq)
{
    for (tim_t tim = 0; tim < TIMER_NUMOF; tim++) {
        if (timer_config[tim].irqn != irq) {
            continue;
        }

        for (unsigned channel = 0; channel < TIMER_CHANNEL_NUMOF; channel++) {
            uint32_t mask = TIM_INTFR_CC1IF << channel;
            if (!(dev(tim)->INTFR & dev(tim)->DMAINTENR & mask)) {
                continue;
            }

            /* Disarm before the callback, which may rearm this channel. */
            dev(tim)->DMAINTENR &= ~mask;
            dev(tim)->INTFR = ~mask;
            if (isr_ctx[tim].cb) {
                isr_ctx[tim].cb(isr_ctx[tim].arg, channel);
            }
        }
        return;
    }
}
