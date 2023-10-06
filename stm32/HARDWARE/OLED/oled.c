#include "stm32f10x.h"  
#include "iic.h"
#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"


//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   



//�������� ��д��
//�������	data: ����	cmd: �Ƿ�Ϊ����OLED_CMD��OLED_DATA
void oled_write_byte(uint8_t data,uint8_t cmd)
{
	if(cmd)
	{
		// д����
		iic_send_data(0x78,0x40,&data,1);
	}
	else 
	{
		// д����
		iic_send_data(0x78,0x00,&data,1);
	}
}


// ��������
void oled_set_pos(unsigned char x, unsigned char y) 
{ 	oled_write_byte(0xb0+y,OLED_CMD);
	oled_write_byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	oled_write_byte((x&0x0f),OLED_CMD); 
}

// ����OLED��ʾ    
void oled_display_on(void)
{
	oled_write_byte(0X8D,OLED_CMD);  //SET DCDC����
	oled_write_byte(0X14,OLED_CMD);  //DCDC ON
	oled_write_byte(0XAF,OLED_CMD);  //DISPLAY ON
}

// �ر�OLED��ʾ     
void oled_display_off(void)
{
	oled_write_byte(0X8D,OLED_CMD);  //SET DCDC����
	oled_write_byte(0X10,OLED_CMD);  //DCDC OFF
	oled_write_byte(0XAE,OLED_CMD);  //DISPLAY OFF
}

// ���� 
void oled_clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_write_byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
		oled_write_byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
		oled_write_byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)oled_write_byte(0,OLED_DATA); 
	} //������ʾ
}


//��������	ָ��λ����ʾһ���ַ�
//�������	x: 0~127	y: 0~63	chr: ��ʾ�ַ�	size:ѡ������ 16/12
void oled_show_char(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0,i=0;	
	c=chr-' ';//�õ�ƫ�ƺ��ֵ			
	if(x>Max_Column-1){x=0;y=y+2;}
	if(Char_Size ==16)
	{
		oled_set_pos(x,y);	
		for(i=0;i<8;i++)
		oled_write_byte(F8X16[c*16+i],OLED_DATA);
		oled_set_pos(x,y+1);
		for(i=0;i<8;i++)
		oled_write_byte(F8X16[c*16+i+8],OLED_DATA);
	}
	else {	
		oled_set_pos(x,y);
		for(i=0;i<6;i++)
		oled_write_byte(F6x8[c][i],OLED_DATA);
		
	}
}

//m^n����
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//��������	��ʾ����
//�������	x,y: �������	 len :���ֵ�λ��	num:��ֵ	len :���ֵ�λ��	size:�����С 		  
void oled_show_num(u8 x,u8 y,u32 num,u8 len,u8 size2)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				oled_show_char(x+(size2/2)*t,y,' ',size2);
				continue;
			}else 
				enshow=1; 
			 
		}
		oled_show_char(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 

// ��ʾһ���ַ��Ŵ�
void oled_show_string(u8 x,u8 y,const char *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		
		oled_show_char(x,y,chr[j],Char_Size);
		x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}

// ��ʾ����
void oled_show_chinese(u8 x,u8 y,u8 no)
{      			    
	u8 t,adder=0;
	oled_set_pos(x,y);	
	for(t=0;t<16;t++)
	{
			oled_write_byte(Hzk[2*no][t],OLED_DATA);
			adder+=1;
	}	
	oled_set_pos(x,y+1);	
	for(t=0;t<16;t++)
	{	
		oled_write_byte(Hzk[2*no+1][t],OLED_DATA);
		adder+=1;
	}					
}

//��������	��ʾBMPͼƬ128��64
//�������	x,y:�������,x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
	unsigned int j=0;
	unsigned char x,y;

	if(y1%8==0) y=y1/8;      
	else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		oled_set_pos(x0,y);
		for(x=x0;x<x1;x++)
		{      
			oled_write_byte(BMP[j++],OLED_DATA);	    	
		}
	}
} 

// ��ʼ��OLED			    
void oled_init(void)
{ 	
	iic_init();

	delay_ms(800);
	oled_write_byte(0xAE,OLED_CMD);//--display off
	oled_write_byte(0x00,OLED_CMD);//---set low column address
	oled_write_byte(0x10,OLED_CMD);//---set high column address
	oled_write_byte(0x40,OLED_CMD);//--set start line address  
	oled_write_byte(0xB0,OLED_CMD);//--set page address
	oled_write_byte(0x81,OLED_CMD); // contract control
	oled_write_byte(0xFF,OLED_CMD);//--128   
	oled_write_byte(0xA1,OLED_CMD);//set segment remap 
	oled_write_byte(0xA6,OLED_CMD);//--normal / reverse
	oled_write_byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	oled_write_byte(0x3F,OLED_CMD);//--1/32 duty
	oled_write_byte(0xC8,OLED_CMD);//Com scan direction
	oled_write_byte(0xD3,OLED_CMD);//-set display offset
	oled_write_byte(0x00,OLED_CMD);//
	
	oled_write_byte(0xD5,OLED_CMD);//set osc division
	oled_write_byte(0x80,OLED_CMD);//
	
	oled_write_byte(0xD8,OLED_CMD);//set area color mode off
	oled_write_byte(0x05,OLED_CMD);//
	
	oled_write_byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	oled_write_byte(0xF1,OLED_CMD);//
	
	oled_write_byte(0xDA,OLED_CMD);//set com pin configuartion
	oled_write_byte(0x12,OLED_CMD);//
	
	oled_write_byte(0xDB,OLED_CMD);//set Vcomh
	oled_write_byte(0x30,OLED_CMD);//
	
	oled_write_byte(0x8D,OLED_CMD);//set charge pump enable
	oled_write_byte(0x14,OLED_CMD);//
	
	oled_write_byte(0xAF,OLED_CMD);//--turn on oled panel
}  
