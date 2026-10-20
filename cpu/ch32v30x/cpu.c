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

void cpu_init(void)
{
    riscv_init();

    early_init();
    periph_init();
}
