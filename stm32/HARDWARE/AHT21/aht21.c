#include "stm32f10x.h"
#include "aht21.h"
#include "iic.h"
#include "Delay.h"
#include <stdio.h>

// ��ʼ����ʪ�ȴ�����
void aht21_init(void)
{
	iic_init();
}

// ��ȡ��ʪ������ 
// ��ʽ: ʪ�� ʪ��С�� �¶� �¶�С��
void aht21_read_data(uint8_t* data)
{
	uint8_t temp_data[7];
	uint64_t value;
	
	iic_read_data_no_mem(AHT21_ADDR,temp_data,1);
	if((temp_data[0]&0x18) != 0x18)
	{
		printf("0x18 error\r\n");
	}
	delay_ms(15);
	
	temp_data[0]=0xac;
	temp_data[1]=0x33;
	temp_data[2]=0x00;
	iic_send_data_no_mem(AHT21_ADDR,temp_data,3);
	delay_ms(85);
	
	do
	{	
		//printf("�ȴ�������...\r\n");
		delay_ms(10);
		iic_read_data_no_mem(AHT21_ADDR,temp_data,1);
	}
	while((temp_data[0]&(1<<7))==1);
	
	iic_read_data_no_mem(AHT21_ADDR,temp_data,7);
	

	// ת��ʪ��
	value=temp_data[1];
	value=value<<8;
	value+=temp_data[2];
	value=value<<8;
	value+=temp_data[3];
	value=value>>4;
	value=value*10000/1048576;
	data[0]=value/100;
	data[1]=value%100;
	
	
	// ת���¶�
	value=temp_data[3]&0x0F;
	value=value<<8;
	value+=temp_data[4];
	value=value<<8;
	value+=temp_data[5];
	value=value*20000/1048576-5000;
	data[2]=value/100;
	data[3]=value%100;
}
