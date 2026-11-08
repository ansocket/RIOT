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
#define EXTI0_IRQn              (22)    /**< EXTI line 0 */
#define EXTI1_IRQn              (23)    /**< EXTI line 1 */
#define EXTI2_IRQn              (24)    /**< EXTI line 2 */
#define EXTI3_IRQn              (25)    /**< EXTI line 3 */
#define EXTI4_IRQn              (26)    /**< EXTI line 4 */
#define EXTI9_5_IRQn            (39)    /**< EXTI lines 5-9 */
#define TIM1_CC_IRQn            (43)    /**< TIM1 capture/compare interrupt */
#define TIM2_IRQn               (44)    /**< TIM2 global interrupt */
#define TIM3_IRQn               (45)    /**< TIM3 global interrupt */
#define TIM4_IRQn               (46)    /**< TIM4 global interrupt */
#define USART1_IRQn             (53)    /**< USART1 global interrupt */
#define USART2_IRQn             (54)    /**< USART2 global interrupt */
#define USART3_IRQn             (55)    /**< USART3 global interrupt */
#define EXTI15_10_IRQn          (56)    /**< EXTI lines 10-15 */
#define TIM8_CC_IRQn            (62)    /**< TIM8 capture/compare interrupt */
#define TIM5_IRQn               (66)    /**< TIM5 global interrupt */
#define UART4_IRQn              (68)    /**< UART4 global interrupt */
#define UART5_IRQn              (69)    /**< UART5 global interrupt */
#define UART6_IRQn              (87)    /**< UART6 global interrupt */
#define UART7_IRQn              (88)    /**< UART7 global interrupt */
#define UART8_IRQn              (89)    /**< UART8 global interrupt */
#define TIM9_CC_IRQn            (93)    /**< TIM9 capture/compare interrupt */
#define TIM10_CC_IRQn           (97)    /**< TIM10 capture/compare interrupt */
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
 * @brief   Alternate function I/O
 */
typedef struct {
    volatile uint32_t ECR;          /**< Event control */
    volatile uint32_t PCFR1;        /**< Remap 1 */
    volatile uint32_t EXTICR[4];    /**< External interrupt configuration */
    uint32_t RESERVED0;             /**< Reserved */
    volatile uint32_t PCFR2;        /**< Remap 2 */
} AFIO_TypeDef;

/**
 * @brief   External interrupt/event controller
 */
typedef struct {
    volatile uint32_t INTENR;       /**< Interrupt enable */
    volatile uint32_t EVENR;        /**< Event enable */
    volatile uint32_t RTENR;        /**< Rising edge trigger enable */
    volatile uint32_t FTENR;        /**< Falling edge trigger enable */
    volatile uint32_t SWIEVR;       /**< Software interrupt event */
    volatile uint32_t INTFR;        /**< Interrupt flag */
} EXTI_TypeDef;

/**
 * @brief   General purpose/advanced timer (16 bit values at 32 bit intervals)
 */
typedef struct {
    volatile uint32_t CTLR1;        /**< Control 1 */
    volatile uint32_t CTLR2;        /**< Control 2 */
    volatile uint32_t SMCFGR;       /**< Slave mode configuration */
    volatile uint32_t DMAINTENR;    /**< DMA/interrupt enable */
    volatile uint32_t INTFR;        /**< Interrupt flags, cleared by writing zero */
    volatile uint32_t SWEVGR;       /**< Software event generation */
    volatile uint32_t CHCTLR1;      /**< Channel control 1 */
    volatile uint32_t CHCTLR2;      /**< Channel control 2 */
    volatile uint32_t CCER;         /**< Capture/compare enable */
    volatile uint32_t CNT;          /**< Counter */
    volatile uint32_t PSC;          /**< Prescaler */
    volatile uint32_t ATRLR;        /**< Auto-reload */
    volatile uint32_t RPTCR;        /**< Repetition counter, TIM1/8/9/10 only */
    volatile uint32_t CHCVR[4];     /**< Channel 1-4 capture/compare values */
} TIM_TypeDef;

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

/** @brief I2C controller (STM32F1-compatible register layout) */
typedef struct {
    volatile uint32_t CTLR1;
    volatile uint32_t CTLR2;
    volatile uint32_t OADDR1;
    volatile uint32_t OADDR2;
    volatile uint32_t DATAR;
    volatile uint32_t STAR1;
    volatile uint32_t STAR2;
    volatile uint32_t CKCFGR;
    volatile uint32_t RTR;
} I2C_TypeDef;

/**
 * @name    Peripheral base addresses
 * @{
 */
#define RCC_BASE                (0x40021000UL)  /**< RCC */
#define AFIO_BASE               (0x40010000UL)  /**< AFIO */
#define EXTI_BASE               (0x40010400UL)  /**< EXTI */
#define GPIOA_BASE              (0x40010800UL)  /**< GPIO port A */
#define GPIO_PORT_OFFSET        (0x400UL)       /**< Distance between ports */
#define TIM1_BASE               (0x40012C00UL)  /**< TIM1 (APB2) */
#define TIM2_BASE               (0x40000000UL)  /**< TIM2 (APB1) */
#define TIM3_BASE               (0x40000400UL)  /**< TIM3 (APB1) */
#define TIM4_BASE               (0x40000800UL)  /**< TIM4 (APB1) */
#define TIM5_BASE               (0x40000C00UL)  /**< TIM5 (APB1) */
#define TIM8_BASE               (0x40013400UL)  /**< TIM8 (APB2) */
#define TIM9_BASE               (0x40014C00UL)  /**< TIM9 (APB2) */
#define TIM10_BASE              (0x40015000UL)  /**< TIM10 (APB2) */
#define USART1_BASE             (0x40013800UL)  /**< USART1 (APB2) */
#define USART2_BASE             (0x40004400UL)  /**< USART2 (APB1) */
#define USART3_BASE             (0x40004800UL)  /**< USART3 (APB1) */
#define UART4_BASE              (0x40004C00UL)  /**< UART4 (APB1) */
#define UART5_BASE              (0x40005000UL)  /**< UART5 (APB1) */
#define UART6_BASE              (0x40001800UL)  /**< UART6 (APB1) */
#define UART7_BASE              (0x40001C00UL)  /**< UART7 (APB1) */
#define UART8_BASE              (0x40002000UL)  /**< UART8 (APB1) */
#define I2C1_BASE              (0x40005400UL)  /**< I2C1 (APB1) */
#define I2C2_BASE              (0x40005800UL)  /**< I2C2 (APB1) */
/** @} */

/**
 * @name    Peripheral instances
 * @{
 */
#define RCC                     ((RCC_TypeDef *)RCC_BASE)
#define AFIO                    ((AFIO_TypeDef *)AFIO_BASE)
#define EXTI                    ((EXTI_TypeDef *)EXTI_BASE)
#define TIM1                    ((TIM_TypeDef *)TIM1_BASE)
#define TIM2                    ((TIM_TypeDef *)TIM2_BASE)
#define TIM3                    ((TIM_TypeDef *)TIM3_BASE)
#define TIM4                    ((TIM_TypeDef *)TIM4_BASE)
#define TIM5                    ((TIM_TypeDef *)TIM5_BASE)
#define TIM8                    ((TIM_TypeDef *)TIM8_BASE)
#define TIM9                    ((TIM_TypeDef *)TIM9_BASE)
#define TIM10                   ((TIM_TypeDef *)TIM10_BASE)
#define GPIOA                   ((GPIO_TypeDef *)(GPIOA_BASE + 0 * GPIO_PORT_OFFSET))
#define GPIOB                   ((GPIO_TypeDef *)(GPIOA_BASE + 1 * GPIO_PORT_OFFSET))
#define GPIOC                   ((GPIO_TypeDef *)(GPIOA_BASE + 2 * GPIO_PORT_OFFSET))
#define GPIOD                   ((GPIO_TypeDef *)(GPIOA_BASE + 3 * GPIO_PORT_OFFSET))
#define GPIOE                   ((GPIO_TypeDef *)(GPIOA_BASE + 4 * GPIO_PORT_OFFSET))
#define USART1                  ((USART_TypeDef *)USART1_BASE)
#define USART2                  ((USART_TypeDef *)USART2_BASE)
#define USART3                  ((USART_TypeDef *)USART3_BASE)
#define UART4                   ((USART_TypeDef *)UART4_BASE)
#define UART5                   ((USART_TypeDef *)UART5_BASE)
#define UART6                   ((USART_TypeDef *)UART6_BASE)
#define UART7                   ((USART_TypeDef *)UART7_BASE)
#define UART8                   ((USART_TypeDef *)UART8_BASE)
#define I2C1                    ((I2C_TypeDef *)I2C1_BASE)
#define I2C2                    ((I2C_TypeDef *)I2C2_BASE)
/** @} */

/**
 * @name    RCC clock control bits
 * @{
 */
#define RCC_CTLR_HSION          (1UL << 0)      /**< HSI enable */
#define RCC_CTLR_HSIRDY         (1UL << 1)      /**< HSI ready */
#define RCC_CTLR_HSEON          (1UL << 16)     /**< HSE enable */
#define RCC_CTLR_HSERDY         (1UL << 17)     /**< HSE ready */
#define RCC_CTLR_HSEBYP         (1UL << 18)     /**< HSE bypass */
#define RCC_CTLR_CSSON          (1UL << 19)     /**< Clock security system enable */
#define RCC_CTLR_PLLON          (1UL << 24)     /**< PLL enable */
#define RCC_CTLR_PLLRDY         (1UL << 25)     /**< PLL ready */
#define RCC_CTLR_PLL2ON         (1UL << 26)     /**< PLL2 enable */
#define RCC_CTLR_PLL3ON         (1UL << 28)     /**< PLL3 enable */
#define RCC_CFGR0_SW_Msk        (3UL << 0)      /**< System clock switch */
#define RCC_CFGR0_SWS_Msk       (3UL << 2)      /**< System clock switch status */
#define RCC_CFGR0_HPRE_Msk      (0xFUL << 4)    /**< AHB prescaler */
#define RCC_CFGR0_PPRE1_Msk     (7UL << 8)      /**< APB1 prescaler */
#define RCC_CFGR0_PPRE2_Msk     (7UL << 11)     /**< APB2 prescaler */
#define RCC_CFGR0_PLLSRC_HSE   (1UL << 16)     /**< PLL source is HSE */
#define RCC_CFGR0_PLLXTPRE_Msk  (1UL << 17)     /**< HSE PLL prescaler */
#define RCC_CFGR0_PLLMUL_Msk    (0xFUL << 18)   /**< PLL multiplier */
#define RCC_CFGR0_SW_PLL        (2UL << 0)      /**< PLL as system clock */
#define RCC_CFGR0_SWS_PLL       (2UL << 2)      /**< PLL is system clock */
#define RCC_CFGR0_HPRE_DIV1     (0UL << 4)
#define RCC_CFGR0_PPRE1_DIV1    (0UL << 8)
#define RCC_CFGR0_PPRE1_DIV2    (4UL << 8)
#define RCC_CFGR0_PPRE2_DIV1    (0UL << 11)
#ifdef CH32V30x_D8
#define RCC_CFGR0_PLLMUL(mul)   (((((mul) == 18) ? 15 : ((mul) - 2)) & 0xFUL) << 18)
#else
/* On CH32V30x variants with the extended PLL encoding, x18 is code 0. */
#define RCC_CFGR0_PLLMUL(mul)   (((((mul) == 18) ? 0 : ((mul) >= 15 ? (mul) - 1 : (mul) - 2)) & 0xFUL) << 18)
#endif
/** @} */

/**
 * @name    RCC clock enable bits
 * @{
 */
#define RCC_APB2PCENR_AFIOEN    (1UL << 0)      /**< AFIO */
#define RCC_APB2PCENR_IOPAEN    (1UL << 2)      /**< GPIOA, B-E follow */
#define RCC_APB2PCENR_TIM1EN    (1UL << 11)     /**< TIM1 */
#define RCC_APB2PCENR_TIM8EN    (1UL << 13)     /**< TIM8 */
#define RCC_APB2PCENR_USART1EN  (1UL << 14)     /**< USART1 */
#define RCC_APB2PCENR_TIM9EN    (1UL << 19)     /**< TIM9 */
#define RCC_APB2PCENR_TIM10EN   (1UL << 20)     /**< TIM10 */
#define RCC_APB1PCENR_TIM2EN    (1UL << 0)      /**< TIM2 */
#define RCC_APB1PCENR_TIM3EN    (1UL << 1)      /**< TIM3 */
#define RCC_APB1PCENR_TIM4EN    (1UL << 2)      /**< TIM4 */
#define RCC_APB1PCENR_TIM5EN    (1UL << 3)      /**< TIM5 */
#define RCC_APB1PCENR_USART2EN  (1UL << 17)     /**< USART2 */
#define RCC_APB1PCENR_USART3EN  (1UL << 18)     /**< USART3 */
#define RCC_APB1PCENR_UART4EN   (1UL << 19)     /**< UART4 */
#define RCC_APB1PCENR_UART5EN   (1UL << 20)     /**< UART5 */
#define RCC_APB1PCENR_UART6EN   (1UL << 25)     /**< UART6 */
#define RCC_APB1PCENR_UART7EN   (1UL << 26)     /**< UART7 */
#define RCC_APB1PCENR_UART8EN   (1UL << 27)     /**< UART8 */
#define RCC_APB1PCENR_I2C1EN    (1UL << 21)     /**< I2C1 */
#define RCC_APB1PCENR_I2C2EN    (1UL << 22)     /**< I2C2 */
/** @} */

/**
 * @name    Timer register bits
 * @{
 */
#define TIM_CTLR1_CEN           (1UL << 0)     /**< Counter enable */
#define TIM_DMAINTENR_CC1IE     (1UL << 1)     /**< Channel 1 interrupt enable */
#define TIM_INTFR_CC1IF         (1UL << 1)     /**< Channel 1 compare flag */
#define TIM_SWEVGR_UG           (1UL << 0)     /**< Update generation */
#define TIM_SWEVGR_CC1G         (1UL << 1)     /**< Channel 1 compare generation */
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
