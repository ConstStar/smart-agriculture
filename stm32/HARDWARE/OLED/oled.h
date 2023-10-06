
//////////////////////////////////////////////////////////////////////////////////	 
//  ��������  : 0.96OLEDģ����ʾ����(STM32F407ZET6ϵ��)
//				���� 0.96OLED��ʾ��_STM32F103C8_IIC_V1.0 ����ֲ
//              ˵��: 
//              ----------------------------------------------------------------
//              VCC	�� 3.3v
//				��������ͬicc.h
//              ----------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////

#ifndef __OLED_H
#define __OLED_H	


#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  

 		     
#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����


//�������� ��д��
//�������	data: ����	cmd: �Ƿ�Ϊ����OLED_CMD��OLED_DATA
void oled_write_byte(uint8_t data,uint8_t cmd);


// ����OLED��ʾ    
void oled_display_on(void);


// �ر�OLED��ʾ
void oled_display_off(void);


// ��ʼ��OLED
void oled_init(void);

// ����
void oled_clear(void);


//��������	ָ��λ����ʾһ���ַ�
//�������	x: 0~127	y: 0~63	chr: ��ʾ�ַ�	size:ѡ������ 16/12
void oled_show_char(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);


//��������	��ʾ����
//�������	x,y: �������	 len :���ֵ�λ��	num:��ֵ	len :���ֵ�λ��	size:�����С
void oled_show_num(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);

// ��ʾһ���ַ��Ŵ�
void oled_show_string(uint8_t x,uint8_t y, const char *p,uint8_t Char_Size);

// ��������
void oled_set_pos(unsigned char x, unsigned char y);

// ��ʾ����
void oled_show_chinese(uint8_t x,uint8_t y,uint8_t no);


//��������	��ʾBMPͼƬ128��64
//�������	x,y:�������,x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);

#endif  
	 



