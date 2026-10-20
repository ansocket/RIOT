/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup         cpu_gd32v
 * @{
 *
 * @file
 * @brief           GD32V CPU initialization
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 */
#include "kernel_init.h"
#include "stdio_uart.h"
#include "periph/init.h"
#include "irq_arch.h"
#include "periph_cpu.h"
#include "periph_conf.h"

#define ENABLE_DEBUG 0
#include "debug.h"

extern void __libc_init_array(void);

void periph_clk_en(bus_t bus, uint32_t mask)
{
    if (bus == APB1) {
        RCC->APB1PCENR |= mask;
    }
    else {
        RCC->APB2PCENR |= mask;
    }
}

void periph_clk_dis(bus_t bus, uint32_t mask)
{
    if (bus == APB1) {
        RCC->APB1PCENR &= ~mask;
    }
    else {
        RCC->APB2PCENR &= ~mask;
    }
}

uint32_t periph_apb_clk(bus_t bus)
{
    return (bus == APB1) ? CLOCK_APB1 : CLOCK_APB2;
}

void gpio_init_af(gpio_t pin, gpio_af_t af)
{
    unsigned port_num = pin >> 4;
    unsigned pin_num = pin & 0xf;
    GPIO_TypeDef *port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * GPIO_PORT_OFFSET);

    periph_clk_en(APB2, RCC_APB2PCENR_AFIOEN | (RCC_APB2PCENR_IOPAEN << port_num));

    volatile uint32_t *cfgr = (pin_num < 8) ? &port->CFGLR : &port->CFGHR;
    unsigned shift = (pin_num & 0x7) * 4;

    *cfgr = (*cfgr & ~(0xfUL << shift)) | ((uint32_t)af << shift);

    /* the output data register selects pull-up or pull-down for inputs */
    if (af == GPIO_AF_IN_PU) {
        port->BSHR = 1UL << pin_num;
    }
}

/**
 * @brief   Put the clock tree into its reset state (HSI, all prescalers 1)
 *
 * The clock tree is not guaranteed to be in reset state, e.g. when the
 * firmware is started by a debugger or a bootloader without a system reset.
 */
static void _clock_init(void)
{
    RCC->CTLR |= RCC_CTLR_HSION;
    while (!(RCC->CTLR & RCC_CTLR_HSIRDY)) {}

    /* switch SYSCLK to HSI, reset prescalers and MCO */
    RCC->CFGR0 = 0;
    while (RCC->CFGR0 & RCC_CFGR0_SWS_Msk) {}

    RCC->CTLR &= ~(RCC_CTLR_HSEON | RCC_CTLR_CSSON | RCC_CTLR_PLLON |
                   RCC_CTLR_PLL2ON | RCC_CTLR_PLL3ON);
    RCC->CTLR &= ~RCC_CTLR_HSEBYP;

    /* disable and clear all clock interrupts */
    RCC->INTR = 0x00FF0000;
}

void cpu_init(void)
{
    _clock_init();

    riscv_init();

    early_init();
    periph_init();
}
