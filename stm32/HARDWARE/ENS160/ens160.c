#include "stm32f10x.h"
#include "ens160.h"
#include "iic.h"
#include "Delay.h"
#include <stdio.h>

void ENS160_OPMODE_Set(uint8_t Mode)
{
    iic_send_data(ENS160_ADDR, ENS160_OPMODE, &Mode, 1);
}

// INT�жϿ����ú���
void ENS160_CONFIG_INTERRUPT_Set(uint8_t Config)
{
    // ������Ч�� 01x3x56x λ
    Config |= 0x02;
    iic_send_data(ENS160_ADDR, ENS160_CONFIG, &Config, 1);
}

void ENS160_Init()
{
    ENS160_OPMODE_Set(ENS160_OPMODE_STANDBY);
    delay_ms(20);
    ENS160_CONFIG_INTERRUPT_Set(0);
    delay_ms(20);
}

void ENS160_Get_DEVICE_STATUS(uint8_t *Dev_Status)
{
    iic_read_data(ENS160_ADDR, ENS160_DEVICE_STATUS, Dev_Status, 1);
}

//void ENS160_Get_DATA_AQI(uint8_t *AQI)
//{
//    iic_read_data(ENS160_ADDR, ENS160_DATA_AQI, AQI, 1);

//    *AQI = *AQI & 0x07; // ��������Ч�ĵ�3λ
//    if (*AQI > 5) *AQI = 5;
//}

//void ENS160_Get_DATA_TVOC(uint16_t *TVOC)
//{
//    u8 Buf[2];

//    iic_read_data(ENS160_ADDR, ENS160_DATA_TVOC, Buf, 2);

//    *TVOC = Buf[1]; // add clear var
//    *TVOC = (*TVOC << 8) | Buf[0];
//}

// ��ȡ������̼����
// ����ֵ	0 ��ȡ�ɹ�		-1 ��ȡʧ��
int ENS160_Get_DATA_ECO2(uint16_t *ECO2)
{
    u8 Buf[2];
	uint8_t state;
	
	// ׼������
    ENS160_OPMODE_Set(ENS160_OPMODE_STANDBY);
	
	// �ȴ�׼������
	ENS160_Get_DEVICE_STATUS(&state);
	for(int i=0;i<5 && (state&0x02)==0 ;++i)
	{
		delay_ms(20);
		ENS160_Get_DEVICE_STATUS(&state);
	}
	
	// �������׼������
	if(state&0x02)
	{
		iic_read_data(ENS160_ADDR, ENS160_DATA_ECO2, Buf, 2);
	
		*ECO2 = Buf[1]; // add clear var
		*ECO2 = (*ECO2 << 8) | Buf[0];
		return 0;
	}
	else
	{
		return -1;
	}
}
