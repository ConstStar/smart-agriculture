#include "stm32f10x.h"
#include "sys.h"
#include "iic.h"
#include "delay.h"
#include <stdio.h>

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif


uint8_t iic_sda_read(void)
{
	return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);
}


void iic_sda_write(uint8_t n)
{
	if(n)
		GPIO_SetBits(GPIOA,GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	
	delay_us(5);
}

void iic_scl_write(uint8_t n)
{
	if(n)
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
	else
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
	
	delay_us(5);
}


//IIC总线的初始化  SCL--PB8  SDA--PB9
void iic_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	iic_sda_write(1);	
	iic_scl_write(1);
}

// 设置引脚为输出模式
void iic_sda_output_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}


// 设置引脚为输入模式
void iic_sda_input_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

//IIC总线开始信号
void iic_send_start(void)
{
	//设置SDA引脚为输出模式
	iic_sda_output_mode();
	
	// 确保为高电平
	iic_sda_write(1);	
	iic_scl_write(1);
	
	//先把SDA引脚拉低
	iic_sda_write(0);	
	//再把SCL引脚拉低
	iic_scl_write(0);
}

//IIC总线停止信号
void iic_send_stop(void)
{
	//设置SDA引脚为输出模式
	iic_sda_output_mode();
	
	// 确保为低电平
	iic_sda_write(0);	
	iic_scl_write(0);
	
	//先把SCL引脚拉高
	iic_scl_write(1);
	//再把SDA引脚拉高
	iic_sda_write(1);	
}

//主机发送数据（从机读取数据）
void iic_send_bytes(uint8_t data)  
{
	uint8_t i = 0;
	
	//设置SDA引脚为输出模式
	iic_sda_output_mode();
	
	// 确保为低电平
	iic_sda_write(0);	
	iic_scl_write(0);
	
	for(i=0;i<8;++i)
	{
		iic_sda_write(data&(1<<7));	
		iic_scl_write(1);
		iic_scl_write(0);	
		data <<= 1;
	}
	
	iic_sda_write(0);
}

//主机读取数据（从机发送数据）
uint8_t iic_read_bytes(void)
{
	uint8_t i = 0;
	uint8_t data = 0; 
	
	//设置SDA引脚为输入模式
	iic_sda_input_mode();
	
	// 确保为低电平
	iic_scl_write(0);

	
	for(i=0;i<8;++i)
	{
		iic_scl_write(1);
		data <<= 1;
		data |= iic_sda_read();
		iic_scl_write(0);	
	}
	
	return data;
}

//主机等待从机应答  返回0 说明从机应答   返回1 说明从机没应答
uint8_t iic_read_ack(void)
{
	uint8_t ack;
	
	//设置SDA引脚为输入模式
	iic_sda_input_mode();
	
	// 确保为低电平
	iic_scl_write(0);
	
	iic_scl_write(1);
	ack = iic_sda_read();
	iic_scl_write(0);
	
	return ack;
}

//从机发送数据，主机接收数据并发送应答信号
void  iic_send_ack(uint8_t ack)
{
	//设置SDA引脚为输出模式
	iic_sda_output_mode();
	
	// 确保为低电平
	iic_scl_write(0);
	iic_sda_write(0);
	
	
	iic_sda_write(ack);	
	iic_scl_write(1);
	iic_scl_write(0);
	iic_sda_write(0);
}

// 一次完整的iic写入数据 (设备有寄存器地址)
void iic_send_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len)
{
	int i=0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	iic_send_start();
	iic_send_bytes(dev_addr);
	if(iic_read_ack() == 1)
	{
		printf("dev_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	iic_send_bytes(mem_addr);
	if(iic_read_ack() == 1)
	{
		printf("mem_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	for(i=0;i<len;++i)
	{
		iic_send_bytes(buf[i]);
		if(iic_read_ack() == 1)
		{
			printf("buf no ack\r\n");
			iic_send_stop();
			return;
		}
	}
	
	iic_send_stop();
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}

// 一次完整的iic读取数据 (设备有寄存器地址)
void iic_read_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len)
{
	int i=0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	iic_send_start();
	iic_send_bytes(dev_addr);
	if(iic_read_ack() == 1)
	{
		printf("dev_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	iic_send_bytes(mem_addr);
	if(iic_read_ack() == 1)
	{
		printf("mem_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	iic_send_start();
	iic_send_bytes(dev_addr|0x01);
	if(iic_read_ack() == 1)
	{
		printf("dev_addr|0x01 no ack\r\n");
		iic_send_stop();
		return;
	}
	
	for(i=0;i<len-1;++i)
	{
		buf[i]=iic_read_bytes();
		iic_send_ack(0);
	}
	buf[i]=iic_read_bytes();
	iic_send_ack(1);
	
	iic_send_stop();
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}

// 一次完整的iic写入数据 (设备无寄存器地址)
void iic_send_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len)
{
	int i=0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	iic_send_start();
	iic_send_bytes(dev_addr);
	if(iic_read_ack() == 1)
	{
		printf("dev_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	for(i=0;i<len;++i)
	{
		iic_send_bytes(buf[i]);
		if(iic_read_ack() == 1)
		{
			printf("mem_addr no ack\r\n");
			iic_send_stop();
			return;
		}
	}
	
	iic_send_stop();
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}

// 一次完整的iic读取数据 (设备无寄存器地址)
void iic_read_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len)
{
	int i=0;

#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	iic_send_start();
	iic_send_bytes(dev_addr|0x01);
	if(iic_read_ack() == 1)
	{
		printf("dev_addr|0x01 no ack\r\n");
		iic_send_stop();
		return;
	}
	
	for(i=0;i<len-1;++i)
	{
		buf[i]=iic_read_bytes();
		iic_send_ack(0);
	}
	buf[i]=iic_read_bytes();
	iic_send_ack(1);
	
	iic_send_stop();
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}
