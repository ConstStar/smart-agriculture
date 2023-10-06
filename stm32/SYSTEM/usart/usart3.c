#include "stm32f10x.h"                  // Device header
#include "sys.h"
#include "usart3.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

// 0、在默认状态下，接受的数据放到usart3_buf里	当usart3_buf为空时收到符号+切换到事件状态
// 1、在事件状态下，接受的数据放到mqtt_even_buf里	当收到换行符，切换到默认状态
uint8_t mqtt_even_state = 0;


uint8_t usart3_buf[RX_DATA_SIZE];
uint8_t usart3_rx_count = 0;


char mqtt_receive_message[RX_DATA_SIZE];		// mqtt接受的消息
uint8_t mqtt_connect_state = 0;					// mqtt的连接状态 0：未连接 1：连接


char esp8266_buf[RX_DATA_SIZE];
uint8_t esp8266_cnt = 0;

void usart3_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);
}

void usart3_send_byte(uint8_t byte)
{
	USART_SendData(USART3, byte);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void usart3_send_array(uint8_t *array, uint16_t length)
{
	uint16_t i;
	for (i = 0; i < length; i ++)
	{
		usart3_send_byte(array[i]);
	}
}

void usart3_send_string(const char *string)
{
	uint8_t i;
	for (i = 0; string[i] != '\0'; i ++)
	{
		usart3_send_byte(string[i]);
	}
}

uint32_t usart3_pow(uint32_t x, uint32_t y)
{
	uint32_t result = 1;
	while (y --)
	{
		result *= x;
	}
	return result;
}

void usart3_send_number(uint32_t number, uint8_t length)
{
	uint8_t i;
	for (i = 0; i < length; i ++)
	{
		usart3_send_byte(number / usart3_pow(10, length - i - 1) % 10 + '0');
	}
}

void usart3_printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	usart3_send_string(String);
}

uint8_t usart3_get_rx_count(void)
{
	return usart3_rx_count;
}

const uint8_t* usart3_get_rx_data(void)
{
	return usart3_buf;
}


void USART3_IRQHandler(void)
{
	uint8_t recv_data = 0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		
		recv_data = USART_ReceiveData(USART3);
//		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
//		
//		esp8266_buf[esp8266_cnt++] = recv_data;
		
		//printf("%c",recv_data);
		if(usart3_rx_count >= RX_DATA_SIZE)
		{
			usart3_rx_count = 0;
		}
		
		
		if(recv_data == '+' && usart3_rx_count == 0)
		{
			mqtt_even_state = 1;
		}
		else if(recv_data == '\r')
		{
			// 忽略
		}
		else if(recv_data == '\n')	
		{
			if(mqtt_even_state == 1)
			{
				usart3_buf[usart3_rx_count] = '\0';
				if(strstr((char *)usart3_buf,"MQTTSUBRECV:")==(char*)usart3_buf)
				{
					char *p;
					
					// 找到第3个逗号
					p=strstr((char *)usart3_buf,",");
					p=strstr(p+1,",");
					p=strstr(p+1,",");
					p = p+1;
					
					// 如果字符串不为空 也就不是测试指令（"心跳包"）
					if(p[0] != '\0')
						strcpy(mqtt_receive_message,p);
				}
				else if(strstr((char *)usart3_buf,"MQTTDISCONNECTED:")==(char*)usart3_buf)
				{
					mqtt_connect_state = 0;
				}
				
				usart3_rx_count = 0;
				mqtt_even_state = 0;
			}
			else
			{
				usart3_buf[usart3_rx_count] = '\0';
			
				if(esp8266_cnt+usart3_rx_count+2>RX_DATA_SIZE-1)
				{
					esp8266_cnt = 0;
					esp8266_buf[0] = '\0';
				}

				strcat((char*)esp8266_buf,(char*)usart3_buf);
				strcat((char*)esp8266_buf,"\r\n");
				esp8266_cnt += usart3_rx_count+2;
				
				usart3_rx_count = 0;
			}
			
		}
		else
		{
			
			// 如果指令长度符合条件就记录下来，否则忽略本次内容 因为需要多存一个结束符所以要-1
			// 否则长度过长则忽略 防止串口被刷爆
			if(usart3_rx_count<RX_DATA_SIZE-1)
			{
				usart3_buf[usart3_rx_count] = recv_data;	
				usart3_rx_count++;
			}
			
		}
		
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
	
}
