#ifndef __AHT21_H
#define __AHT21_H


#define AHT21_ADDR 0x70

// 初始化温湿度传感器
void aht21_init(void);

// 获取温湿度数据 
// 格式: 湿度 湿度小数 温度 温度小数
void aht21_read_data(uint8_t* data);

#endif
