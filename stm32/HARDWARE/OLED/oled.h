
//////////////////////////////////////////////////////////////////////////////////	 
//  功能描述  : 0.96OLED模块演示例程(STM32F407ZET6系列)
//				来自 0.96OLED显示屏_STM32F103C8_IIC_V1.0 的移植
//              说明: 
//              ----------------------------------------------------------------
//              VCC	： 3.3v
//				其他引脚同icc.h
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

 		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//功能描述 字写入
//输入参数	data: 数据	cmd: 是否为命令OLED_CMD、OLED_DATA
void oled_write_byte(uint8_t data,uint8_t cmd);


// 开启OLED显示    
void oled_display_on(void);


// 关闭OLED显示
void oled_display_off(void);


// 初始化OLED
void oled_init(void);

// 清屏
void oled_clear(void);


//功能描述	指定位置显示一个字符
//输入参数	x: 0~127	y: 0~63	chr: 显示字符	size:选择字体 16/12
void oled_show_char(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);


//功能描述	显示数字
//输入参数	x,y: 起点坐标	 len :数字的位数	num:数值	len :数字的位数	size:字体大小
void oled_show_num(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);

// 显示一个字符号串
void oled_show_string(uint8_t x,uint8_t y, const char *p,uint8_t Char_Size);

// 坐标设置
void oled_set_pos(unsigned char x, unsigned char y);

// 显示汉字
void oled_show_chinese(uint8_t x,uint8_t y,uint8_t no);


//功能描述	显示BMP图片128×64
//输入参数	x,y:起点坐标,x的范围0～127，y为页的范围0～7
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);

#endif  
	 



