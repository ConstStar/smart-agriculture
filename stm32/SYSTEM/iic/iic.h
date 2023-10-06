
//////////////////////////////////////////////////////////////////////////////////	 
//  ��������  : IIC��ʾ����(STM32F407ZET6ϵ��)
//              ˵��: 
//              ----------------------------------------------------------------
//              SCL �� PA2
//				SDA	�� PA1
//              ----------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////


#ifndef __IIC_H
#define __IIC_H


//IIC���ߵĳ�ʼ��
void iic_init(void);

//IIC���߿�ʼ�ź�
void iic_send_start(void);

//IIC����ֹͣ�ź�
void iic_send_stop(void);

//�����������ݣ��ӻ���ȡ���ݣ�
void iic_send_bytes(uint8_t data);

//������ȡ���ݣ��ӻ��������ݣ�
uint8_t iic_read_bytes(void);


//�����ȴ��ӻ�Ӧ��  ����0 ˵���ӻ�Ӧ��   ����1 ˵���ӻ�ûӦ��
uint8_t iic_read_ack(void);


//�ӻ��������ݣ������������ݲ�����Ӧ���ź�
void  iic_send_ack(uint8_t ack);

// һ��������iicд������ (�豸�мĴ�����ַ)
void iic_send_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len);

// һ��������iic��ȡ���� (�豸�мĴ�����ַ)
void iic_read_data(uint8_t dev_addr,uint8_t mem_addr,uint8_t *buf,uint8_t len);

// һ��������iicд������ (�豸�޼Ĵ�����ַ)
void iic_send_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len);
	
// һ��������iic��ȡ���� (�豸�޼Ĵ�����ַ)
void iic_read_data_no_mem(uint8_t dev_addr,uint8_t *buf,uint8_t len);

#endif
