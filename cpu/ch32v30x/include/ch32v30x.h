/*
 * SPDX-FileCopyrightText: 2026 ansocket
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup         cpu_ch32v30x
 * @{
 *
 * @file
 * @brief           CH32V30x peripheral register definitions
 *
 * Only the peripherals used by RIOT so far are described here.
 *
 * @author          ansocket
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    PFIC interrupt numbers
 * @{
 */
#define USART1_IRQn             (53)    /**< USART1 global interrupt */
#define USART2_IRQn             (54)    /**< USART2 global interrupt */
#define USART3_IRQn             (55)    /**< USART3 global interrupt */
#define UART4_IRQn              (68)    /**< UART4 global interrupt */
#define UART5_IRQn              (69)    /**< UART5 global interrupt */
#define UART6_IRQn              (87)    /**< UART6 global interrupt */
#define UART7_IRQn              (88)    /**< UART7 global interrupt */
#define UART8_IRQn              (89)    /**< UART8 global interrupt */
/** @} */

/**
 * @brief   Reset and clock control
 */
typedef struct {
    volatile uint32_t CTLR;         /**< Clock control */
    volatile uint32_t CFGR0;        /**< Clock configuration 0 */
    volatile uint32_t INTR;         /**< Clock interrupt */
    volatile uint32_t APB2PRSTR;    /**< APB2 peripheral reset */
    volatile uint32_t APB1PRSTR;    /**< APB1 peripheral reset */
    volatile uint32_t AHBPCENR;     /**< AHB peripheral clock enable */
    volatile uint32_t APB2PCENR;    /**< APB2 peripheral clock enable */
    volatile uint32_t APB1PCENR;    /**< APB1 peripheral clock enable */
    volatile uint32_t BDCTLR;       /**< Backup domain control */
    volatile uint32_t RSTSCKR;      /**< Control/status */
} RCC_TypeDef;

/**
 * @brief   General purpose I/O port
 */
typedef struct {
    volatile uint32_t CFGLR;        /**< Configuration of pins 0-7 */
    volatile uint32_t CFGHR;        /**< Configuration of pins 8-15 */
    volatile uint32_t INDR;         /**< Input data */
    volatile uint32_t OUTDR;        /**< Output data */
    volatile uint32_t BSHR;         /**< Bit set/reset */
    volatile uint32_t BCR;          /**< Bit reset */
    volatile uint32_t LCKR;         /**< Configuration lock */
} GPIO_TypeDef;

/**
 * @brief   Universal synchronous asynchronous receiver transmitter
 */
typedef struct {
    volatile uint32_t STATR;        /**< Status */
    volatile uint32_t DATAR;        /**< Data */
    volatile uint32_t BRR;          /**< Baud rate */
    volatile uint32_t CTLR1;        /**< Control 1 */
    volatile uint32_t CTLR2;        /**< Control 2 */
    volatile uint32_t CTLR3;        /**< Control 3 */
    volatile uint32_t GPR;          /**< Guard time and prescaler */
} USART_TypeDef;

/**
 * @name    Peripheral base addresses
 * @{
 */
#define RCC_BASE                (0x40021000UL)  /**< RCC */
#define GPIOA_BASE              (0x40010800UL)  /**< GPIO port A */
#define GPIO_PORT_OFFSET        (0x400UL)       /**< Distance between ports */
#define USART1_BASE             (0x40013800UL)  /**< USART1 (APB2) */
#define USART2_BASE             (0x40004400UL)  /**< USART2 (APB1) */
#define USART3_BASE             (0x40004800UL)  /**< USART3 (APB1) */
#define UART4_BASE              (0x40004C00UL)  /**< UART4 (APB1) */
#define UART5_BASE              (0x40005000UL)  /**< UART5 (APB1) */
#define UART6_BASE              (0x40001800UL)  /**< UART6 (APB1) */
#define UART7_BASE              (0x40001C00UL)  /**< UART7 (APB1) */
#define UART8_BASE              (0x40002000UL)  /**< UART8 (APB1) */
/** @} */

/**
 * @name    Peripheral instances
 * @{
 */
#define RCC                     ((RCC_TypeDef *)RCC_BASE)
#define USART1                  ((USART_TypeDef *)USART1_BASE)
#define USART2                  ((USART_TypeDef *)USART2_BASE)
#define USART3                  ((USART_TypeDef *)USART3_BASE)
#define UART4                   ((USART_TypeDef *)UART4_BASE)
#define UART5                   ((USART_TypeDef *)UART5_BASE)
#define UART6                   ((USART_TypeDef *)UART6_BASE)
#define UART7                   ((USART_TypeDef *)UART7_BASE)
#define UART8                   ((USART_TypeDef *)UART8_BASE)
/** @} */

/**
 * @name    RCC clock control bits
 * @{
 */
#define RCC_CTLR_HSION          (1UL << 0)      /**< HSI enable */
#define RCC_CTLR_HSIRDY         (1UL << 1)      /**< HSI ready */
#define RCC_CTLR_HSEON          (1UL << 16)     /**< HSE enable */
#define RCC_CTLR_HSEBYP         (1UL << 18)     /**< HSE bypass */
#define RCC_CTLR_CSSON          (1UL << 19)     /**< Clock security system enable */
#define RCC_CTLR_PLLON          (1UL << 24)     /**< PLL enable */
#define RCC_CTLR_PLL2ON         (1UL << 26)     /**< PLL2 enable */
#define RCC_CTLR_PLL3ON         (1UL << 28)     /**< PLL3 enable */
#define RCC_CFGR0_SWS_Msk       (3UL << 2)      /**< System clock switch status */
/** @} */

/**
 * @name    RCC clock enable bits
 * @{
 */
#define RCC_APB2PCENR_AFIOEN    (1UL << 0)      /**< AFIO */
#define RCC_APB2PCENR_IOPAEN    (1UL << 2)      /**< GPIOA, B-E follow */
#define RCC_APB2PCENR_USART1EN  (1UL << 14)     /**< USART1 */
#define RCC_APB1PCENR_USART2EN  (1UL << 17)     /**< USART2 */
#define RCC_APB1PCENR_USART3EN  (1UL << 18)     /**< USART3 */
#define RCC_APB1PCENR_UART4EN   (1UL << 19)     /**< UART4 */
#define RCC_APB1PCENR_UART5EN   (1UL << 20)     /**< UART5 */
#define RCC_APB1PCENR_UART6EN   (1UL << 25)     /**< UART6 */
#define RCC_APB1PCENR_UART7EN   (1UL << 26)     /**< UART7 */
#define RCC_APB1PCENR_UART8EN   (1UL << 27)     /**< UART8 */
/** @} */

/**
 * @name    USART register bits
 * @{
 */
#define USART_STATR_ORE         (1UL << 3)      /**< Overrun error */
#define USART_STATR_RXNE        (1UL << 5)      /**< Read data register not empty */
#define USART_STATR_TC          (1UL << 6)      /**< Transmission complete */
#define USART_STATR_TXE         (1UL << 7)      /**< Transmit data register empty */
#define USART_CTLR1_RE          (1UL << 2)      /**< Receiver enable */
#define USART_CTLR1_TE          (1UL << 3)      /**< Transmitter enable */
#define USART_CTLR1_RXNEIE      (1UL << 5)      /**< RXNE interrupt enable */
#define USART_CTLR1_UE          (1UL << 13)     /**< USART enable */
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
