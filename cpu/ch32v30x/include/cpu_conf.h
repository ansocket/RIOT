/*
 * SPDX-FileCopyrightText: 2020 Koen Zandberg <koen@bergzand.net>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup         cpu_gd32v
 * @{
 *
 * @file
 * @brief           CPU specific configuration options
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 */

#include "cpu_conf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPU_DEFAULT_IRQ_PRIO (0xFF)

/**
 * @brief   Number of PFIC interrupts (16 core + 88 peripheral interrupts)
 */
#define PFIC_NUM_INTERRUPTS             (104)

/**
 * @brief   The gd32v doesn't use the MIE CSR, but uses the CLIC instead
 */
#define HAVE_CSR_MIE                    (0)

/**
 * @name Flashpage settings
 * @{
 */
#define FLASHPAGE_SIZE                  (1024U)
#define FLASHPAGE_NUMOF                 (128U)
#define FLASHPAGE_WRITE_BLOCK_SIZE      (2U)
#define FLASHPAGE_WRITE_BLOCK_ALIGNMENT (4U)
#define CPU_FLASH_BASE                  0x00000000
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
