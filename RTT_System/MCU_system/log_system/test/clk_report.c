/**
 * @file clk_report.c
 * @brief STM32H7 完整版 - 含 PLL1/PLL2/PLL3 全部输出解析
 */

#include "clk_report.h"
#include "stm32h7xx_hal.h"
#include "log_system.h"

#ifndef LOG_RAW
#include <stdio.h>
#define LOG_RAW(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

/* ============================================================
 *  辅助打印
 * ============================================================ */
static void log_mhz(const char *name, uint32_t hz)
{
    uint32_t mi = hz / 1000000U;
    uint32_t mf = (hz % 1000000U) / 10000U;
    LOG_RAW(" %-9s: %3lu.%02lu MHz\r\n",
            name, (unsigned long)mi, (unsigned long)mf);
}

static void log_off(const char *name)
{
    LOG_RAW(" %-9s: OFF\r\n", name);
}

/* ============================================================
 *  基础总线时钟
 * ============================================================ */
static uint32_t get_apb1(void)   { return HAL_RCC_GetPCLK1Freq(); }
static uint32_t get_apb2(void)   { return HAL_RCC_GetPCLK2Freq(); }
static uint32_t get_hclk(void)   { return HAL_RCC_GetHCLKFreq(); }
static uint32_t get_sysclk(void) { return HAL_RCC_GetSysClockFreq(); }

static uint32_t get_apb1_tim(void)
{
    uint32_t p = get_apb1();
    return (p < get_hclk()) ? (p * 2U) : p;
}

static uint32_t get_apb2_tim(void)
{
    uint32_t p = get_apb2();
    return (p < get_hclk()) ? (p * 2U) : p;
}

static uint32_t get_apb4(void)
{
    static const uint8_t div_tbl[] = {1,1,1,1,2,4,8,16};
#if defined(RCC_D3CFGR_D3PPRE)
    uint32_t idx = (RCC->D3CFGR & RCC_D3CFGR_D3PPRE)
                    >> RCC_D3CFGR_D3PPRE_Pos;
#elif defined(RCC_SRDCFGR_SRDPPRE)
    uint32_t idx = (RCC->SRDCFGR & RCC_SRDCFGR_SRDPPRE)
                    >> RCC_SRDCFGR_SRDPPRE_Pos;
#else
    uint32_t idx = 0;
#endif
    return get_hclk() / div_tbl[idx & 0x07U];
}

/* ============================================================
 *  PLL 输入源频率 (PLL1/2/3 共享 PLLSRC)
 * ============================================================ */
static uint32_t get_pll_src(void)
{
    switch (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC)
    {
        case RCC_PLLCKSELR_PLLSRC_HSI: return HSI_VALUE;
        case RCC_PLLCKSELR_PLLSRC_CSI: return CSI_VALUE;
        case RCC_PLLCKSELR_PLLSRC_HSE: return HSE_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  PLL1 P / Q / R 输出
 * ============================================================ */
static uint32_t get_pll1_vco(void)
{
    uint32_t src = get_pll_src();
    uint32_t m = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1)
                  >> RCC_PLLCKSELR_DIVM1_Pos;
    if (m == 0U) m = 1U;
    uint32_t n = ((RCC->PLL1DIVR & RCC_PLL1DIVR_N1)
                   >> RCC_PLL1DIVR_N1_Pos) + 1U;
    return (uint32_t)(((uint64_t)src / m) * n);
}

static uint32_t get_pll1_p(void)
{
    uint32_t p = ((RCC->PLL1DIVR & RCC_PLL1DIVR_P1)
                   >> RCC_PLL1DIVR_P1_Pos) + 1U;
    return get_pll1_vco() / p;
}

static uint32_t get_pll1_q(void)
{
    uint32_t q = ((RCC->PLL1DIVR & RCC_PLL1DIVR_Q1)
                   >> RCC_PLL1DIVR_Q1_Pos) + 1U;
    return get_pll1_vco() / q;
}

static uint32_t get_pll1_r(void)
{
    uint32_t r = ((RCC->PLL1DIVR & RCC_PLL1DIVR_R1)
                   >> RCC_PLL1DIVR_R1_Pos) + 1U;
    return get_pll1_vco() / r;
}

/* ============================================================
 *  PLL2 P / Q / R 输出
 * ============================================================ */
static uint32_t get_pll2_vco(void)
{
    uint32_t src = get_pll_src();
    uint32_t m = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM2)
                  >> RCC_PLLCKSELR_DIVM2_Pos;
    if (m == 0U) m = 1U;
    uint32_t n = ((RCC->PLL2DIVR & RCC_PLL2DIVR_N2)
                   >> RCC_PLL2DIVR_N2_Pos) + 1U;
    return (uint32_t)(((uint64_t)src / m) * n);
}

static uint32_t get_pll2_p(void)
{
    /* 检查 PLL2 是否已使能 */
    if (!(RCC->CR & RCC_CR_PLL2ON)) return 0;
    uint32_t p = ((RCC->PLL2DIVR & RCC_PLL2DIVR_P2)
                   >> RCC_PLL2DIVR_P2_Pos) + 1U;
    return get_pll2_vco() / p;
}

static uint32_t get_pll2_q(void)
{
    if (!(RCC->CR & RCC_CR_PLL2ON)) return 0;
    uint32_t q = ((RCC->PLL2DIVR & RCC_PLL2DIVR_Q2)
                   >> RCC_PLL2DIVR_Q2_Pos) + 1U;
    return get_pll2_vco() / q;
}

static uint32_t get_pll2_r(void)
{
    if (!(RCC->CR & RCC_CR_PLL2ON)) return 0;
    uint32_t r = ((RCC->PLL2DIVR & RCC_PLL2DIVR_R2)
                   >> RCC_PLL2DIVR_R2_Pos) + 1U;
    return get_pll2_vco() / r;
}

/* ============================================================
 *  PLL3 P / Q / R 输出
 * ============================================================ */
static uint32_t get_pll3_vco(void)
{
    uint32_t src = get_pll_src();
    uint32_t m = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM3)
                  >> RCC_PLLCKSELR_DIVM3_Pos;
    if (m == 0U) m = 1U;
    uint32_t n = ((RCC->PLL3DIVR & RCC_PLL3DIVR_N3)
                   >> RCC_PLL3DIVR_N3_Pos) + 1U;
    return (uint32_t)(((uint64_t)src / m) * n);
}

static uint32_t get_pll3_p(void)
{
    if (!(RCC->CR & RCC_CR_PLL3ON)) return 0;
    uint32_t p = ((RCC->PLL3DIVR & RCC_PLL3DIVR_P3)
                   >> RCC_PLL3DIVR_P3_Pos) + 1U;
    return get_pll3_vco() / p;
}

static uint32_t get_pll3_q(void)
{
    if (!(RCC->CR & RCC_CR_PLL3ON)) return 0;
    uint32_t q = ((RCC->PLL3DIVR & RCC_PLL3DIVR_Q3)
                   >> RCC_PLL3DIVR_Q3_Pos) + 1U;
    return get_pll3_vco() / q;
}

static uint32_t get_pll3_r(void)
{
    if (!(RCC->CR & RCC_CR_PLL3ON)) return 0;
    uint32_t r = ((RCC->PLL3DIVR & RCC_PLL3DIVR_R3)
                   >> RCC_PLL3DIVR_R3_Pos) + 1U;
    return get_pll3_vco() / r;
}

/* ============================================================
 *  SPI 内核时钟
 *  SPI1/2/3 -> SPI123SEL (PLL1_Q / PLL2_P / PLL3_P / HSI / CSI / HSE)
 *  SPI4/5   -> SPI45SEL  (APB / PLL2_Q / PLL3_Q / HSI / CSI / HSE)
 *  SPI6     -> SPI6SEL   (APB4 / PLL2_Q / PLL3_Q / HSI / CSI / HSE)
 * ============================================================ */
static uint32_t get_spi123_ker(void)
{
#if defined(RCC_D2CCIP1R_SPI123SEL)
    uint32_t sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_SPI123SEL)
                    >> RCC_D2CCIP1R_SPI123SEL_Pos;
#elif defined(RCC_CDCCIP1R_SPI123SEL)
    uint32_t sel = (RCC->CDCCIP1R & RCC_CDCCIP1R_SPI123SEL)
                    >> RCC_CDCCIP1R_SPI123SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_pll1_q();
        case 1: return get_pll2_p();
        case 2: return get_pll3_p();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return HSE_VALUE;
        default: return 0;
    }
}

static uint32_t get_spi45_ker(void)
{
#if defined(RCC_D2CCIP1R_SPI45SEL)
    uint32_t sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_SPI45SEL)
                    >> RCC_D2CCIP1R_SPI45SEL_Pos;
#elif defined(RCC_CDCCIP1R_SPI45SEL)
    uint32_t sel = (RCC->CDCCIP1R & RCC_CDCCIP1R_SPI45SEL)
                    >> RCC_CDCCIP1R_SPI45SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb2();
        case 1: return get_pll2_q();
        case 2: return get_pll3_q();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return HSE_VALUE;
        default: return 0;
    }
}

static uint32_t get_spi6_ker(void)
{
#if defined(RCC_D3CCIPR_SPI6SEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_SPI6SEL)
                    >> RCC_D3CCIPR_SPI6SEL_Pos;
#elif defined(RCC_SRDCCIPR_SPI6SEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_SPI6SEL)
                    >> RCC_SRDCCIPR_SPI6SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb4();
        case 1: return get_pll2_q();
        case 2: return get_pll3_q();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return HSE_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  USART/UART 内核时钟
 *  USART1/6     -> USART16SEL  (APB2/PLL2_Q/PLL3_Q/HSI/CSI/LSE)
 *  USART2-8     -> USART234578SEL (APB1/PLL2_Q/PLL3_Q/HSI/CSI/LSE)
 *  LPUART1      -> LPUART1SEL (APB4/PLL2_Q/PLL3_Q/HSI/CSI/LSE)
 * ============================================================ */
static uint32_t get_usart16_ker(void)
{
#if defined(RCC_D2CCIP2R_USART16SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_USART16SEL)
                    >> RCC_D2CCIP2R_USART16SEL_Pos;
#elif defined(RCC_CDCCIP2R_USART16910SEL)
    uint32_t sel = (RCC->CDCCIP2R & RCC_CDCCIP2R_USART16910SEL)
                    >> RCC_CDCCIP2R_USART16910SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb2();
        case 1: return get_pll2_q();
        case 2: return get_pll3_q();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return LSE_VALUE;
        default: return 0;
    }
}

static uint32_t get_usart234578_ker(void)
{
#if defined(RCC_D2CCIP2R_USART28SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_USART28SEL)
                    >> RCC_D2CCIP2R_USART28SEL_Pos;
#elif defined(RCC_CDCCIP2R_USART234578SEL)
    uint32_t sel = (RCC->CDCCIP2R & RCC_CDCCIP2R_USART234578SEL)
                    >> RCC_CDCCIP2R_USART234578SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb1();
        case 1: return get_pll2_q();
        case 2: return get_pll3_q();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return LSE_VALUE;
        default: return 0;
    }
}

static uint32_t get_lpuart1_ker(void)
{
#if defined(RCC_D3CCIPR_LPUART1SEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_LPUART1SEL)
                    >> RCC_D3CCIPR_LPUART1SEL_Pos;
#elif defined(RCC_SRDCCIPR_LPUART1SEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_LPUART1SEL)
                    >> RCC_SRDCCIPR_LPUART1SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb4();
        case 1: return get_pll2_q();
        case 2: return get_pll3_q();
        case 3: return HSI_VALUE;
        case 4: return CSI_VALUE;
        case 5: return LSE_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  I2C 内核时钟
 *  I2C1/2/3 -> I2C123SEL (APB1 / PLL3_R / HSI / CSI)
 *  I2C4     -> I2C4SEL   (APB4 / PLL3_R / HSI / CSI)
 * ============================================================ */
static uint32_t get_i2c123_ker(void)
{
#if defined(RCC_D2CCIP2R_I2C123SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_I2C123SEL)
                    >> RCC_D2CCIP2R_I2C123SEL_Pos;
#elif defined(RCC_CDCCIP2R_I2C123SEL)
    uint32_t sel = (RCC->CDCCIP2R & RCC_CDCCIP2R_I2C123SEL)
                    >> RCC_CDCCIP2R_I2C123SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb1();
        case 1: return get_pll3_r();
        case 2: return HSI_VALUE;
        case 3: return CSI_VALUE;
        default: return 0;
    }
}

static uint32_t get_i2c4_ker(void)
{
#if defined(RCC_D3CCIPR_I2C4SEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_I2C4SEL)
                    >> RCC_D3CCIPR_I2C4SEL_Pos;
#elif defined(RCC_SRDCCIPR_I2C4SEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_I2C4SEL)
                    >> RCC_SRDCCIPR_I2C4SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb4();
        case 1: return get_pll3_r();
        case 2: return HSI_VALUE;
        case 3: return CSI_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  SDMMC 内核时钟
 *  SDMMC1/2 -> SDMMCSEL (PLL1_Q / PLL2_R)
 * ============================================================ */
static uint32_t get_sdmmc_ker(void)
{
#if defined(RCC_D1CCIPR_SDMMCSEL)
    uint32_t sel = (RCC->D1CCIPR & RCC_D1CCIPR_SDMMCSEL)
                    >> RCC_D1CCIPR_SDMMCSEL_Pos;
#elif defined(RCC_CDCCIPR_SDMMCSEL)
    uint32_t sel = (RCC->CDCCIPR & RCC_CDCCIPR_SDMMCSEL)
                    >> RCC_CDCCIPR_SDMMCSEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_pll1_q();
        case 1: return get_pll2_r();
        default: return 0;
    }
}

/* ============================================================
 *  FDCAN 内核时钟
 *  FDCAN -> FDCANSEL (HSE / PLL1_Q / PLL2_Q)
 * ============================================================ */
static uint32_t get_fdcan_ker(void)
{
#if defined(RCC_D2CCIP1R_FDCANSEL)
    uint32_t sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_FDCANSEL)
                    >> RCC_D2CCIP1R_FDCANSEL_Pos;
#elif defined(RCC_CDCCIP1R_FDCANSEL)
    uint32_t sel = (RCC->CDCCIP1R & RCC_CDCCIP1R_FDCANSEL)
                    >> RCC_CDCCIP1R_FDCANSEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return HSE_VALUE;
        case 1: return get_pll1_q();
        case 2: return get_pll2_q();
        default: return 0;
    }
}

/* ============================================================
 *  USB 内核时钟
 *  USB -> USBSEL (DISABLE / PLL1_Q / PLL3_Q / HSI48)
 * ============================================================ */
static uint32_t get_usb_ker(void)
{
#if defined(RCC_D2CCIP2R_USBSEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_USBSEL)
                    >> RCC_D2CCIP2R_USBSEL_Pos;
#elif defined(RCC_CDCCIP2R_USBSEL)
    uint32_t sel = (RCC->CDCCIP2R & RCC_CDCCIP2R_USBSEL)
                    >> RCC_CDCCIP2R_USBSEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return 0;                   /* Disabled */
        case 1: return get_pll1_q();
        case 2: return get_pll3_q();
        case 3: return 48000000U;           /* HSI48 */
        default: return 0;
    }
}

/* ============================================================
 *  ADC 内核时钟
 *  ADC -> ADCSEL (PLL2_P / PLL3_R / PER_CK)
 * ============================================================ */
static uint32_t get_adc_ker(void)
{
#if defined(RCC_D3CCIPR_ADCSEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_ADCSEL)
                    >> RCC_D3CCIPR_ADCSEL_Pos;
#elif defined(RCC_SRDCCIPR_ADCSEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_ADCSEL)
                    >> RCC_SRDCCIPR_ADCSEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_pll2_p();
        case 1: return get_pll3_r();
        case 2: return HSE_VALUE;           /* per_ck 简化 */
        default: return 0;
    }
}

/* ============================================================
 *  QSPI/OCTOSPI 内核时钟
 *  -> QSPISEL / OCTOSPISEL (HCLK3/PLL1_Q/PLL2_R/PER_CK)
 * ============================================================ */
static uint32_t get_qspi_ker(void)
{
#if defined(RCC_D1CCIPR_QSPISEL)
    uint32_t sel = (RCC->D1CCIPR & RCC_D1CCIPR_QSPISEL)
                    >> RCC_D1CCIPR_QSPISEL_Pos;
#elif defined(RCC_CDCCIPR_OCTOSPISEL)
    uint32_t sel = (RCC->CDCCIPR & RCC_CDCCIPR_OCTOSPISEL)
                    >> RCC_CDCCIPR_OCTOSPISEL_Pos;
#elif defined(RCC_D1CCIPR_OCTOSPISEL)
    uint32_t sel = (RCC->D1CCIPR & RCC_D1CCIPR_OCTOSPISEL)
                    >> RCC_D1CCIPR_OCTOSPISEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_hclk();
        case 1: return get_pll1_q();
        case 2: return get_pll2_r();
        case 3: return HSE_VALUE;           /* per_ck */
        default: return 0;
    }
}

/* ============================================================
 *  LPTIM1 内核时钟
 *  -> LPTIM1SEL (APB1/PLL2_P/PLL3_R/LSE/LSI/PER_CK)
 * ============================================================ */
static uint32_t get_lptim1_ker(void)
{
#if defined(RCC_D2CCIP2R_LPTIM1SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_LPTIM1SEL)
                    >> RCC_D2CCIP2R_LPTIM1SEL_Pos;
#elif defined(RCC_CDCCIP2R_LPTIM1SEL)
    uint32_t sel = (RCC->CDCCIP2R & RCC_CDCCIP2R_LPTIM1SEL)
                    >> RCC_CDCCIP2R_LPTIM1SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb1();
        case 1: return get_pll2_p();
        case 2: return get_pll3_r();
        case 3: return LSE_VALUE;
        case 4: return LSI_VALUE;
        case 5: return HSE_VALUE;           /* per_ck */
        default: return 0;
    }
}

/* ============================================================
 *  LPTIM2 内核时钟
 *  -> LPTIM2SEL (APB4/PLL2_P/PLL3_R/LSE/LSI/PER_CK)
 * ============================================================ */
static uint32_t get_lptim2_ker(void)
{
#if defined(RCC_D3CCIPR_LPTIM2SEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_LPTIM2SEL)
                    >> RCC_D3CCIPR_LPTIM2SEL_Pos;
#elif defined(RCC_SRDCCIPR_LPTIM2SEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_LPTIM2SEL)
                    >> RCC_SRDCCIPR_LPTIM2SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb4();
        case 1: return get_pll2_p();
        case 2: return get_pll3_r();
        case 3: return LSE_VALUE;
        case 4: return LSI_VALUE;
        case 5: return HSE_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  LPTIM3/4/5 内核时钟 (与 LPTIM2 相同 mux)
 * ============================================================ */
static uint32_t get_lptim345_ker(void)
{
#if defined(RCC_D3CCIPR_LPTIM345SEL)
    uint32_t sel = (RCC->D3CCIPR & RCC_D3CCIPR_LPTIM345SEL)
                    >> RCC_D3CCIPR_LPTIM345SEL_Pos;
#elif defined(RCC_SRDCCIPR_LPTIM3SEL)
    uint32_t sel = (RCC->SRDCCIPR & RCC_SRDCCIPR_LPTIM3SEL)
                    >> RCC_SRDCCIPR_LPTIM3SEL_Pos;
#else
    uint32_t sel = 0;
#endif
    switch (sel)
    {
        case 0: return get_apb4();
        case 1: return get_pll2_p();
        case 2: return get_pll3_r();
        case 3: return LSE_VALUE;
        case 4: return LSI_VALUE;
        case 5: return HSE_VALUE;
        default: return 0;
    }
}

/* ============================================================
 *  外设使能检测宏
 * ============================================================ */
#define IS_EN(reg, bit)  ((RCC->reg) & (RCC_##reg##_##bit))

/* ============================================================
 *  各外设组上报
 * ============================================================ */
static void report_spi(void)
{
    LOG_RAW("-- SPI -----------\r\n");
#if defined(RCC_APB2ENR_SPI1EN)
    if (IS_EN(APB2ENR, SPI1EN))
        log_mhz("SPI1", get_spi123_ker());
    else log_off("SPI1");
#endif
#if defined(RCC_APB1LENR_SPI2EN)
    if (IS_EN(APB1LENR, SPI2EN))
        log_mhz("SPI2", get_spi123_ker());
    else log_off("SPI2");
#endif
#if defined(RCC_APB1LENR_SPI3EN)
    if (IS_EN(APB1LENR, SPI3EN))
        log_mhz("SPI3", get_spi123_ker());
    else log_off("SPI3");
#endif
#if defined(RCC_APB2ENR_SPI4EN)
    if (IS_EN(APB2ENR, SPI4EN))
        log_mhz("SPI4", get_spi45_ker());
    else log_off("SPI4");
#endif
#if defined(RCC_APB2ENR_SPI5EN)
    if (IS_EN(APB2ENR, SPI5EN))
        log_mhz("SPI5", get_spi45_ker());
    else log_off("SPI5");
#endif
#if defined(RCC_APB4ENR_SPI6EN)
    if (IS_EN(APB4ENR, SPI6EN))
        log_mhz("SPI6", get_spi6_ker());
    else log_off("SPI6");
#endif
}

static void report_usart(void)
{
    LOG_RAW("-- USART/UART ----\r\n");
#if defined(RCC_APB2ENR_USART1EN)
    if (IS_EN(APB2ENR, USART1EN))
        log_mhz("USART1", get_usart16_ker());
    else log_off("USART1");
#endif
#if defined(RCC_APB1LENR_USART2EN)
    if (IS_EN(APB1LENR, USART2EN))
        log_mhz("USART2", get_usart234578_ker());
    else log_off("USART2");
#endif
#if defined(RCC_APB1LENR_USART3EN)
    if (IS_EN(APB1LENR, USART3EN))
        log_mhz("USART3", get_usart234578_ker());
    else log_off("USART3");
#endif
#if defined(RCC_APB1LENR_UART4EN)
    if (IS_EN(APB1LENR, UART4EN))
        log_mhz("UART4", get_usart234578_ker());
    else log_off("UART4");
#endif
#if defined(RCC_APB1LENR_UART5EN)
    if (IS_EN(APB1LENR, UART5EN))
        log_mhz("UART5", get_usart234578_ker());
    else log_off("UART5");
#endif
#if defined(RCC_APB2ENR_USART6EN)
    if (IS_EN(APB2ENR, USART6EN))
        log_mhz("USART6", get_usart16_ker());
    else log_off("USART6");
#endif
#if defined(RCC_APB1LENR_UART7EN)
    if (IS_EN(APB1LENR, UART7EN))
        log_mhz("UART7", get_usart234578_ker());
    else log_off("UART7");
#endif
#if defined(RCC_APB1LENR_UART8EN)
    if (IS_EN(APB1LENR, UART8EN))
        log_mhz("UART8", get_usart234578_ker());
    else log_off("UART8");
#endif
#if defined(RCC_APB4ENR_LPUART1EN)
    if (IS_EN(APB4ENR, LPUART1EN))
        log_mhz("LPUART1", get_lpuart1_ker());
    else log_off("LPUART1");
#endif
}

static void report_i2c(void)
{
    LOG_RAW("-- I2C -----------\r\n");
#if defined(RCC_APB1LENR_I2C1EN)
    if (IS_EN(APB1LENR, I2C1EN))
        log_mhz("I2C1", get_i2c123_ker());
    else log_off("I2C1");
#endif
#if defined(RCC_APB1LENR_I2C2EN)
    if (IS_EN(APB1LENR, I2C2EN))
        log_mhz("I2C2", get_i2c123_ker());
    else log_off("I2C2");
#endif
#if defined(RCC_APB1LENR_I2C3EN)
    if (IS_EN(APB1LENR, I2C3EN))
        log_mhz("I2C3", get_i2c123_ker());
    else log_off("I2C3");
#endif
#if defined(RCC_APB4ENR_I2C4EN)
    if (IS_EN(APB4ENR, I2C4EN))
        log_mhz("I2C4", get_i2c4_ker());
    else log_off("I2C4");
#endif
}

static void report_timer(void)
{
    LOG_RAW("-- TIM (APB2) ----\r\n");
#if defined(RCC_APB2ENR_TIM1EN)
    if (IS_EN(APB2ENR, TIM1EN))
        log_mhz("TIM1", get_apb2_tim());
    else log_off("TIM1");
#endif
#if defined(RCC_APB2ENR_TIM8EN)
    if (IS_EN(APB2ENR, TIM8EN))
        log_mhz("TIM8", get_apb2_tim());
    else log_off("TIM8");
#endif
#if defined(RCC_APB2ENR_TIM15EN)
    if (IS_EN(APB2ENR, TIM15EN))
        log_mhz("TIM15", get_apb2_tim());
    else log_off("TIM15");
#endif
#if defined(RCC_APB2ENR_TIM16EN)
    if (IS_EN(APB2ENR, TIM16EN))
        log_mhz("TIM16", get_apb2_tim());
    else log_off("TIM16");
#endif
#if defined(RCC_APB2ENR_TIM17EN)
    if (IS_EN(APB2ENR, TIM17EN))
        log_mhz("TIM17", get_apb2_tim());
    else log_off("TIM17");
#endif

    LOG_RAW("-- TIM (APB1) ----\r\n");
#if defined(RCC_APB1LENR_TIM2EN)
    if (IS_EN(APB1LENR, TIM2EN))
        log_mhz("TIM2", get_apb1_tim());
    else log_off("TIM2");
#endif
#if defined(RCC_APB1LENR_TIM3EN)
    if (IS_EN(APB1LENR, TIM3EN))
        log_mhz("TIM3", get_apb1_tim());
    else log_off("TIM3");
#endif
#if defined(RCC_APB1LENR_TIM4EN)
    if (IS_EN(APB1LENR, TIM4EN))
        log_mhz("TIM4", get_apb1_tim());
    else log_off("TIM4");
#endif
#if defined(RCC_APB1LENR_TIM5EN)
    if (IS_EN(APB1LENR, TIM5EN))
        log_mhz("TIM5", get_apb1_tim());
    else log_off("TIM5");
#endif
#if defined(RCC_APB1LENR_TIM6EN)
    if (IS_EN(APB1LENR, TIM6EN))
        log_mhz("TIM6", get_apb1_tim());
    else log_off("TIM6");
#endif
#if defined(RCC_APB1LENR_TIM7EN)
    if (IS_EN(APB1LENR, TIM7EN))
        log_mhz("TIM7", get_apb1_tim());
    else log_off("TIM7");
#endif
#if defined(RCC_APB1LENR_TIM12EN)
    if (IS_EN(APB1LENR, TIM12EN))
        log_mhz("TIM12", get_apb1_tim());
    else log_off("TIM12");
#endif
#if defined(RCC_APB1LENR_TIM13EN)
    if (IS_EN(APB1LENR, TIM13EN))
        log_mhz("TIM13", get_apb1_tim());
    else log_off("TIM13");
#endif
#if defined(RCC_APB1LENR_TIM14EN)
    if (IS_EN(APB1LENR, TIM14EN))
        log_mhz("TIM14", get_apb1_tim());
    else log_off("TIM14");
#endif

    LOG_RAW("-- LPTIM ---------\r\n");
#if defined(RCC_APB1LENR_LPTIM1EN)
    if (IS_EN(APB1LENR, LPTIM1EN))
        log_mhz("LPTIM1", get_lptim1_ker());
    else log_off("LPTIM1");
#endif
#if defined(RCC_APB4ENR_LPTIM2EN)
    if (IS_EN(APB4ENR, LPTIM2EN))
        log_mhz("LPTIM2", get_lptim2_ker());
    else log_off("LPTIM2");
#endif
#if defined(RCC_APB4ENR_LPTIM3EN)
    if (IS_EN(APB4ENR, LPTIM3EN))
        log_mhz("LPTIM3", get_lptim345_ker());
    else log_off("LPTIM3");
#endif
#if defined(RCC_APB4ENR_LPTIM4EN)
    if (IS_EN(APB4ENR, LPTIM4EN))
        log_mhz("LPTIM4", get_lptim345_ker());
    else log_off("LPTIM4");
#endif
#if defined(RCC_APB4ENR_LPTIM5EN)
    if (IS_EN(APB4ENR, LPTIM5EN))
        log_mhz("LPTIM5", get_lptim345_ker());
    else log_off("LPTIM5");
#endif
}

static void report_adc(void)
{
    LOG_RAW("-- ADC -----------\r\n");
#if defined(RCC_AHB1ENR_ADC12EN)
    if (IS_EN(AHB1ENR, ADC12EN))
        log_mhz("ADC12", get_adc_ker());
    else log_off("ADC12");
#endif
#if defined(RCC_AHB4ENR_ADC3EN)
    if (IS_EN(AHB4ENR, ADC3EN))
        log_mhz("ADC3", get_adc_ker());
    else log_off("ADC3");
#endif
}

static void report_dma(void)
{
    LOG_RAW("-- DMA -----------\r\n");
#if defined(RCC_AHB1ENR_DMA1EN)
    if (IS_EN(AHB1ENR, DMA1EN))
        log_mhz("DMA1", get_hclk());
    else log_off("DMA1");
#endif
#if defined(RCC_AHB1ENR_DMA2EN)
    if (IS_EN(AHB1ENR, DMA2EN))
        log_mhz("DMA2", get_hclk());
    else log_off("DMA2");
#endif
#if defined(RCC_AHB4ENR_BDMAEN)
    if (IS_EN(AHB4ENR, BDMAEN))
        log_mhz("BDMA", get_hclk());
    else log_off("BDMA");
#endif
#if defined(RCC_AHB1ENR_MDMAEN)
    if (IS_EN(AHB1ENR, MDMAEN))
        log_mhz("MDMA", get_hclk());
    else log_off("MDMA");
#endif
}

static void report_sdmmc(void)
{
    LOG_RAW("-- SDMMC ---------\r\n");
#if defined(RCC_AHB3ENR_SDMMC1EN)
    if (IS_EN(AHB3ENR, SDMMC1EN))
        log_mhz("SDMMC1", get_sdmmc_ker());
    else log_off("SDMMC1");
#endif
#if defined(RCC_AHB2ENR_SDMMC2EN)
    if (IS_EN(AHB2ENR, SDMMC2EN))
        log_mhz("SDMMC2", get_sdmmc_ker());
    else log_off("SDMMC2");
#endif
}

static void report_fdcan(void)
{
    LOG_RAW("-- FDCAN ---------\r\n");
#if defined(RCC_APB1HENR_FDCANEN)
    if (IS_EN(APB1HENR, FDCANEN))
        log_mhz("FDCAN", get_fdcan_ker());
    else log_off("FDCAN");
#endif
}

static void report_usb(void)
{
    LOG_RAW("-- USB -----------\r\n");
#if defined(RCC_AHB1ENR_USB1OTGHSEN)
    if (IS_EN(AHB1ENR, USB1OTGHSEN))
        log_mhz("USB1_HS", get_usb_ker());
    else log_off("USB1_HS");
#endif
#if defined(RCC_AHB1ENR_USB2OTGHSEN)
    if (IS_EN(AHB1ENR, USB2OTGHSEN))
        log_mhz("USB2_HS", get_usb_ker());
    else log_off("USB2_HS");
#endif
}

static void report_eth(void)
{
    LOG_RAW("-- ETH -----------\r\n");
#if defined(RCC_AHB1ENR_ETH1MACEN)
    if (IS_EN(AHB1ENR, ETH1MACEN))
        log_mhz("ETH_MAC", get_hclk());
    else log_off("ETH_MAC");
#endif
}

static void report_qspi(void)
{
    LOG_RAW("-- QSPI/OSPI -----\r\n");
#if defined(RCC_AHB3ENR_QSPIEN)
    if (IS_EN(AHB3ENR, QSPIEN))
        log_mhz("QSPI", get_qspi_ker());
    else log_off("QSPI");
#endif
#if defined(RCC_AHB3ENR_OSPI1EN)
    if (IS_EN(AHB3ENR, OSPI1EN))
        log_mhz("OSPI1", get_qspi_ker());
    else log_off("OSPI1");
#endif
#if defined(RCC_AHB3ENR_OSPI2EN)
    if (IS_EN(AHB3ENR, OSPI2EN))
        log_mhz("OSPI2", get_qspi_ker());
    else log_off("OSPI2");
#endif
}

static void report_fmc(void)
{
    LOG_RAW("-- FMC -----------\r\n");
#if defined(RCC_AHB3ENR_FMCEN)
    if (IS_EN(AHB3ENR, FMCEN))
        log_mhz("FMC", get_hclk());
    else log_off("FMC");
#endif
}

static void report_rng(void)
{
    LOG_RAW("-- RNG/CRC -------\r\n");
#if defined(RCC_AHB2ENR_RNGEN)
    if (IS_EN(AHB2ENR, RNGEN))
        log_mhz("RNG", get_hclk());
    else log_off("RNG");
#endif
#if defined(RCC_AHB4ENR_CRCEN)
    if (IS_EN(AHB4ENR, CRCEN))
        log_mhz("CRC", get_hclk());
    else log_off("CRC");
#endif
}

static void report_dac(void)
{
    LOG_RAW("-- DAC -----------\r\n");
#if defined(RCC_APB1LENR_DAC12EN)
    if (IS_EN(APB1LENR, DAC12EN))
        log_mhz("DAC12", get_apb1());
    else log_off("DAC12");
#endif
}

static void report_gpio(void)
{
    LOG_RAW("-- GPIO ----------\r\n");
    const char *names[] = {
        "GPIOA","GPIOB","GPIOC","GPIOD",
        "GPIOE","GPIOF","GPIOG","GPIOH",
        "GPIOI","GPIOJ","GPIOK"
    };
    for (uint32_t i = 0; i <= 10; i++)
    {
#if defined(RCC_AHB4ENR_GPIOAEN)
        uint32_t bit = RCC_AHB4ENR_GPIOAEN << i;
        if ((RCC->AHB4ENR & bit) && (i <= 10))
            log_mhz(names[i], get_hclk());
        else if (i <= 10)
            log_off(names[i]);
#endif
    }
}

/* ============================================================
 *  PLL 状态概览
 * ============================================================ */
static void report_pll(void)
{
    LOG_RAW("-- PLL Status ----\r\n");

    /* PLL1 */
    if (RCC->CR & RCC_CR_PLL1ON)
    {
        LOG_RAW(" PLL1     : ON\r\n");
        log_mhz("  PLL1_P", get_pll1_p());
        log_mhz("  PLL1_Q", get_pll1_q());
        log_mhz("  PLL1_R", get_pll1_r());
    }
    else
    {
        LOG_RAW(" PLL1     : OFF\r\n");
    }

    /* PLL2 */
    if (RCC->CR & RCC_CR_PLL2ON)
    {
        LOG_RAW(" PLL2     : ON\r\n");
        log_mhz("  PLL2_P", get_pll2_p());
        log_mhz("  PLL2_Q", get_pll2_q());
        log_mhz("  PLL2_R", get_pll2_r());
    }
    else
    {
        LOG_RAW(" PLL2     : OFF\r\n");
    }

    /* PLL3 */
    if (RCC->CR & RCC_CR_PLL3ON)
    {
        LOG_RAW(" PLL3     : ON\r\n");
        log_mhz("  PLL3_P", get_pll3_p());
        log_mhz("  PLL3_Q", get_pll3_q());
        log_mhz("  PLL3_R", get_pll3_r());
    }
    else
    {
        LOG_RAW(" PLL3     : OFF\r\n");
    }
}

/* ============================================================
 *  公开接口
 * ============================================================ */
void CLK_Report_Core(void)
{
    LOG_RAW("====== CORE ======\r\n");
    log_mhz("SYSCLK",   get_sysclk());
    log_mhz("HCLK",     get_hclk());
    log_mhz("APB1",     get_apb1());
    log_mhz("APB1_TIM", get_apb1_tim());
    log_mhz("APB2",     get_apb2());
    log_mhz("APB2_TIM", get_apb2_tim());
    log_mhz("APB4",     get_apb4());
    report_pll();
}

void CLK_Report_Periph(void)
{
    LOG_RAW("====== PERIPH ====\r\n");
    report_spi();
    report_usart();
    report_i2c();
    report_timer();
    report_adc();
    report_dac();
    report_dma();
    report_sdmmc();
    report_fdcan();
    report_usb();
    report_eth();
    report_qspi();
    report_fmc();
    report_rng();
    report_gpio();
}

void CLK_Report_All(void)
{
    CLK_Report_Core();
    CLK_Report_Periph();
    LOG_RAW("==================\r\n");
}