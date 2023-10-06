#include "stm32f10x.h"
#include "bh1750.h"
#include "iic.h"
#include <stdio.h>

//开启光照传感器
void bh1750_power_on(void)
{	
	iic_send_start();
	iic_send_bytes(0x46);
	if(iic_read_ack() == 1)
	{
		printf("bh1750_power_on dev_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	iic_send_bytes(0x01);
	if(iic_read_ack() == 1)
	{
		printf("bh1750_power_on mem_addr no ack\r\n");
		iic_send_stop();
		return;
	}
	
	iic_send_stop();
}

// 读取光照强度数据
double bh1750_read_data(void)
{
	uint8_t temp_data[2];
	iic_read_data(0x46,0x20,temp_data,2);
	return (((uint16_t)temp_data[0]<<7)+(double)(temp_data[1]&0x01)*0.5+(temp_data[1]>>1))/1.2;
}

// 初始化光照传感器
void bh1750_init(void)
{
	iic_init();
	bh1750_power_on();
}
