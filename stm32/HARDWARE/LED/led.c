#include "led.h"

// 初始化状态指示灯
void led_init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;					//输出模式 推挽输出
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;					//输出速率
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_5|GPIO_Pin_6;	//引脚编号
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_5); 							// 默认不亮
	GPIO_SetBits(GPIOA,GPIO_Pin_6); 							// 默认不亮
}
 


// 设置 是否亮绿灯
void led_set_green(uint8_t led)
{
	if(led)
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		
}

// 设置 是否亮黄灯
void led_set_yellow(uint8_t led)
{
	if(led)
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_6);
}
