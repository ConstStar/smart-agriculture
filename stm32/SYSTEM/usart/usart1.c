#include "stm32f10x.h"                  // Device header
#include "sys.h"
#include "usart1.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

char hc14_receive_message[RX_DATA_SIZE];

uint8_t usart1_buf[RX_DATA_SIZE];
uint8_t usart1_rx_count = 0;


void usart1_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

void usart1_send_byte(uint8_t byte)
{
	USART_SendData(USART1, byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void usart1_send_u32(uint32_t data)
{
	usart1_send_byte(data>>24);
	usart1_send_byte(data>>16&0xff);
	usart1_send_byte(data>>8&0xff);
	usart1_send_byte(data&0xff);
}

void usart1_send_array(uint8_t *array, uint16_t length)
{
	uint16_t i;
	for (i = 0; i < length; i ++)
	{
		usart1_send_byte(array[i]);
	}
}

void usart1_send_string(char *string)
{
	uint8_t i;
	for (i = 0; string[i] != '\0'; i ++)
	{
		usart1_send_byte(string[i]);
	}
}

uint32_t usart1_pow(uint32_t x, uint32_t y)
{
	uint32_t result = 1;
	while (y --)
	{
		result *= x;
	}
	return result;
}

void usart1_send_number(uint32_t number, uint8_t length)
{
	uint8_t i;
	for (i = 0; i < length; i ++)
	{
		usart1_send_byte(number / usart1_pow(10, length - i - 1) % 10 + '0');
	}
}

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
	return x;
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif

void usart1_printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	usart1_send_string(String);
}

uint8_t usart1_get_rx_count(void)
{
	return usart1_rx_count;
}

const uint8_t* usart1_get_rx_data(void)
{
	return usart1_buf;
}


void USART1_IRQHandler(void)
{
	uint8_t recv_data = 0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		recv_data = USART_ReceiveData(USART1);
		if(usart1_rx_count >= RX_DATA_SIZE)
		{
			usart1_rx_count = 0;
		}
		
		//printf("TEST\r\n");
		
		if(recv_data == '\r')
		{
			// 忽略
		}
		else if(recv_data == '#')
		{
			usart1_rx_count = 0;	
		}
		else if(recv_data == '\n')	
		{
			memcpy(hc14_receive_message, usart1_buf, usart1_rx_count); 
			hc14_receive_message[usart1_rx_count] = '\0';
		}
		else
		{
			//  如果指令长度符合条件就记录下来，否则忽略本次内容 因为需要多存一个结束符所以要-1
			if(usart1_rx_count<RX_DATA_SIZE-1)
			{
				usart1_buf[usart1_rx_count] = recv_data;	
				usart1_rx_count++;
			}				
		}
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}
