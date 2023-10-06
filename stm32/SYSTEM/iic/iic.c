#include "stm32f10x.h"
#include "sys.h"
#include "iic.h"
#include "delay.h"
#include <stdio.h>

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
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


//IIC���ߵĳ�ʼ��  SCL--PB8  SDA--PB9
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

// ��������Ϊ���ģʽ
void iic_sda_output_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}


// ��������Ϊ����ģʽ
void iic_sda_input_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

//IIC���߿�ʼ�ź�
void iic_send_start(void)
{
	//����SDA����Ϊ���ģʽ
	iic_sda_output_mode();
	
	// ȷ��Ϊ�ߵ�ƽ
	iic_sda_write(1);	
	iic_scl_write(1);
	
	//�Ȱ�SDA��������
	iic_sda_write(0);	
	//�ٰ�SCL��������
	iic_scl_write(0);
}

//IIC����ֹͣ�ź�
void iic_send_stop(void)
{
	//����SDA����Ϊ���ģʽ
	iic_sda_output_mode();
	
	// ȷ��Ϊ�͵�ƽ
	iic_sda_write(0);	
	iic_scl_write(0);
	
	//�Ȱ�SCL��������
	iic_scl_write(1);
	//�ٰ�SDA��������
	iic_sda_write(1);	
}

//�����������ݣ��ӻ���ȡ���ݣ�
void iic_send_bytes(uint8_t data)  
{
	uint8_t i = 0;
	
	//����SDA����Ϊ���ģʽ
	iic_sda_output_mode();
	
	// ȷ��Ϊ�͵�ƽ
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

//������ȡ���ݣ��ӻ��������ݣ�
uint8_t iic_read_bytes(void)
{
	uint8_t i = 0;
	uint8_t data = 0; 
	
	//����SDA����Ϊ����ģʽ
	iic_sda_input_mode();
	
	// ȷ��Ϊ�͵�ƽ
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

//�����ȴ��ӻ�Ӧ��  ����0 ˵���ӻ�Ӧ��   ����1 ˵���ӻ�ûӦ��
uint8_t iic_read_ack(void)
{
	uint8_t ack;
	
	//����SDA����Ϊ����ģʽ
	iic_sda_input_mode();
	
	// ȷ��Ϊ�͵�ƽ
	iic_scl_write(0);
	
	iic_scl_write(1);
	ack = iic_sda_read();
	iic_scl_write(0);
	
	return ack;
}

//�ӻ��������ݣ������������ݲ�����Ӧ���ź�
void  iic_send_ack(uint8_t ack)
{
	//����SDA����Ϊ���ģʽ
	iic_sda_output_mode();
	
	// ȷ��Ϊ�͵�ƽ
	iic_scl_write(0);
	iic_sda_write(0);
	
	
	iic_sda_write(ack);	
	iic_scl_write(1);
	iic_scl_write(0);
	iic_sda_write(0);
}

// һ��������iicд������ (�豸�мĴ�����ַ)
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

// һ��������iic��ȡ���� (�豸�мĴ�����ַ)
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

// һ��������iicд������ (�豸�޼Ĵ�����ַ)
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

// һ��������iic��ȡ���� (�豸�޼Ĵ�����ַ)
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
