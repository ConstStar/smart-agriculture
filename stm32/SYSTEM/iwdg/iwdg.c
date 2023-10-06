#include "sys.h"

#include "stm32f10x_iwdg.h"
#include "iwdg.h"

void iwdg_init(uint16_t s)
{	
	// 使能 预分频寄存器PR和重装载寄存器RLR可写
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	// 设置预分频器值
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	
	// 设置重装载寄存器值
	IWDG_SetReload(s*156.25);
	
	// 把重装载寄存器的值放到计数器中
	IWDG_ReloadCounter();
	
	// 使能 IWDG，然后每来一个脉冲，计数器的值-1，如果不喂狗，等计数器的值减到0，就会产生复位
	IWDG_Enable();	
}

// 喂狗
void iwdg_feed(void)
{
	// 把重装载寄存器的值放到计数器中，喂狗，防止IWDG复位
	// 当计数器的值减到0的时候会产生系统复位
	IWDG_ReloadCounter();
}
