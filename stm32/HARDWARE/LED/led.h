#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

// 初始化状态指示灯
void led_init(void);

// 设置 是否亮绿灯
void led_set_green(uint8_t led);

// 设置 是否亮黄灯
void led_set_yellow(uint8_t led);
		 				    
#endif
