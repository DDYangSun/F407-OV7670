#include "stm32f4xx_conf.h"
#include "Delay.h"

static uint32_t  fac_us=0;
static uint32_t fac_ms=0;

// SysTick LOAD寄存器是24位，最大值为0xFFFFFF = 16777215
#define SYSTICK_LOAD_MAX    0xFFFFFFUL

void Delay_Init(uint8_t  SYSCLK)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us=SYSCLK/8;
    fac_ms=fac_us*1000;
}

void Delay_us(uint32_t nus)
{
	/* fac_us 为 0 说明整个工程没人调用过 Delay_Init()，这里自动补上，
	   否则所有 Delay_us()/Delay_ms() 都会变成空操作（SCCB 时序直接失效） */
	if (fac_us == 0) { Delay_Init((uint8_t)(SystemCoreClock / 1000000)); }

    uint32_t temp;
    uint32_t total_ticks = nus * fac_us;
    
    // 分多次调用，避免SysTick LOAD寄存器溢出（24位限制）
    while(total_ticks > SYSTICK_LOAD_MAX)
    {
        SysTick->LOAD = SYSTICK_LOAD_MAX;
        SysTick->VAL = 0x00;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        do
        {
            temp = SysTick->CTRL;
        }while((temp & 0x01) && !(temp & (1 << 16)));
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTick->VAL = 0X00;
        total_ticks -= SYSTICK_LOAD_MAX;
    }
    
    if(total_ticks > 0)
    {
        SysTick->LOAD = total_ticks;
        SysTick->VAL = 0x00;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        do
        {
            temp = SysTick->CTRL;
        }while((temp & 0x01) && !(temp & (1 << 16)));
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTick->VAL = 0X00;
    }
}

void Delay_ms(uint16_t nms)
{
	/* fac_us 为 0 说明整个工程没人调用过 Delay_Init()，这里自动补上，
	   否则所有 Delay_us()/Delay_ms() 都会变成空操作（SCCB 时序直接失效） */
	if (fac_us == 0) { Delay_Init((uint8_t)(SystemCoreClock / 1000000)); }

    uint32_t temp;
    uint32_t total_ticks = (uint32_t)nms * fac_ms;
    
    // 分多次调用，避免SysTick LOAD寄存器溢出（24位限制）
    while(total_ticks > SYSTICK_LOAD_MAX)
    {
        SysTick->LOAD = SYSTICK_LOAD_MAX;
        SysTick->VAL = 0x00;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        do
        {
            temp = SysTick->CTRL;
        }while((temp & 0x01) && !(temp & (1 << 16)));
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTick->VAL = 0X00;
        total_ticks -= SYSTICK_LOAD_MAX;
    }
    
    if(total_ticks > 0)
    {
        SysTick->LOAD = total_ticks;
        SysTick->VAL = 0x00;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        do
        {
            temp = SysTick->CTRL;
        }while((temp & 0x01) && !(temp & (1 << 16)));
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTick->VAL = 0X00;
    }
}
