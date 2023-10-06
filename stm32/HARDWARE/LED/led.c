#include "led.h"

// ��ʼ��״ָ̬ʾ��
void led_init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;					//���ģʽ �������
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;					//�������
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_5|GPIO_Pin_6;	//���ű��
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_5); 							// Ĭ�ϲ���
	GPIO_SetBits(GPIOA,GPIO_Pin_6); 							// Ĭ�ϲ���
}
 


// ���� �Ƿ����̵�
void led_set_green(uint8_t led)
{
	if(led)
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		
}

// ���� �Ƿ����Ƶ�
void led_set_yellow(uint8_t led)
{
	if(led)
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_6);
}
