#ifndef __AHT21_H
#define __AHT21_H


#define AHT21_ADDR 0x70

// ��ʼ����ʪ�ȴ�����
void aht21_init(void);

// ��ȡ��ʪ������ 
// ��ʽ: ʪ�� ʪ��С�� �¶� �¶�С��
void aht21_read_data(uint8_t* data);

#endif
