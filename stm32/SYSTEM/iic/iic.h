
//////////////////////////////////////////////////////////////////////////////////	 
//  功能描述  : IIC演示例程(STM32F407ZET6系列)
//              说明: 
//              ----------------------------------------------------------------
//              SCL ： PA2
//				SDA	： PA1
//              ----------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////


#ifndef __IIC_H
#define __IIC_H


//IIC总线的初始化
void iic_init(void);

//IIC总线开始信号
void iic_send_start(void);

//IIC总线停止信号
void iic_send_stop(void);

//主机发送数据（从机读取数据）
void iic_send_bytes(uint8_t data);

//主机读取数据（从机发送数据）
uint8_t iic_read_bytes(void);


//主机等待从机应答  返回0 说明从机应答   返回1 说明从机没应答
uint8_t iic_read_ack(void);


//从机发送数据，主机接收数据并发送应答信号
void  iic_send_ack(uint8_t ack);

// 一次完整的iic写入数据 (设备有寄存器地址)
void iic_send_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len);

// 一次完整的iic读取数据 (设备有寄存器地址)
void iic_read_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len);

// 一次完整的iic写入数据 (设备无寄存器地址)
void iic_send_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len);
	
// 一次完整的iic读取数据 (设备无寄存器地址)
void iic_read_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len);

#endif
