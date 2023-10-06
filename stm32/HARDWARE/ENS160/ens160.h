#ifndef __ENS160_H
#define __ENS160_H

#define ENS160_ADDR 0xA6 // IIC写地址
#define ENS160_OPMODE 0x10
#define ENS160_CONFIG 0x11

#define ENS160_OPMODE_DEEP_SLEEP 0x00
#define ENS160_OPMODE_IDLE       0x01
#define ENS160_OPMODE_STANDBY    0x02
#define ENS160_OPMODE_RESET      0xF0

#define ENS160_DEVICE_STATUS 0x20

#define ENS160_DATA_AQI 0x21
#define ENS160_DATA_TVOC 0x22
#define ENS160_DATA_ECO2 0x24

void ENS160_OPMODE_Set(uint8_t Mode);

void ENS160_CONFIG_INTERRUPT_Set(uint8_t Config);

void ENS160_Init(void);

void ENS160_Get_DEVICE_STATUS(uint8_t *Dev_Status);

//void ENS160_Get_DATA_AQI(uint8_t *AQI);

//void ENS160_Get_DATA_TVOC(uint16_t *TVOC);

// 获取二氧化碳数据
// 返回值	0 获取成功		-1 获取失败
int ENS160_Get_DATA_ECO2(uint16_t *ECO2);

#endif
