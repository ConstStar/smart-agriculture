#ifndef __ESP8266_H
#define __ESP8266_H	


#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־


// ��ջ���
void esp8266_clear(void);
 
// ����MQTT��Ϣ
// ����ֵ 	0 ���ͳɹ�		-1 ����ʧ��
int esp8266_send_mqtt_message(char *data);

// ��ʼ��ESP8266
void esp8266_init(void);

// ����mqtt
void esp8266_connect_mqtt(void);

// �ж�esp8266 && mqtt������״̬
// ����ֵ		0 ��������		-1 δ����
int esp8266_get_connected_state(void);

#endif
