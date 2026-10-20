/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg
 * SPDX-FileCopyrightText: 2023 Gunar Schorcht
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_gd32v
 * @{
 *
 * @brief       Implementation of the CPU power management for Gigadevice GD32V
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @}
 */

#include <stdint.h>
#include <stdio.h>


void pm_set(unsigned mode)
{
    (void)mode;
}

void pm_reboot(void)
{
    while (1) {}
}
