#include "stm32f10x.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "esp8266.h"
#include "usart3.h"
#include "delay.h"
#include "includes.h"

#define ESP8266_WIFI_NAME		"test"
#define ESP8266_WIFI_PASSWORD	"1164442003"
 
#define ESP8266_MQTT_ADDR		"39.107.228.202"
#define ESP8266_MQTT_PORT		1883
 
#define MQTT_CLIENT_ID 	"client"		//
#define MQTT_USERNAME  	""				//����
#define MQTT_PASSWORD  	""				//����

#define MQTT_TOPIC_ME	"smart_agriculture_client"
#define MQTT_TOPIC_TA	"smart_agriculture_ui"
 
extern char esp8266_buf[RX_DATA_SIZE];
extern uint8_t esp8266_cnt;
uint8_t esp8266_cntPre = 0;

//mqtt	����״̬ 0��δ���� 1������
extern uint8_t mqtt_connect_state;

//mqtt ���ܵ���Ϣ
extern char mqtt_receive_message[RX_DATA_SIZE];


// ��ջ���
void esp8266_clear(void)
{
 
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
 
}
 
// �ж���Ϣ�Ƿ�������	��Ҫѭ������
int esp8266_wait_receive(void)
{
 
	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־
 
}
 
// ��������
// ����ֵ	0 �˶���ȷ	-1 �˶�����
int esp8266_send_cmd(const char *cmd, char *res)
{
	
	uint8_t time_out = 200;
 
	usart3_send_string(cmd);
	
	while(time_out--)
	{
		if(esp8266_wait_receive() == REV_OK)							//����յ�����
		{
			
			//printf("%s\r\n",esp8266_buf); 
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
#if DEBUG
				if(strstr(cmd,"MQTTSUB") != NULL)
					printf("%s\r\n",esp8266_buf); 
#endif
				esp8266_clear();									//��ջ���
				return 0;
			}
		}
		
#ifdef SYSTEM_SUPPORT_OS	 	
		delay_us(10000);
#else		
		delay_ms(10);
#endif
	
	}
	return -1;
 
}
 
 
// ����MQTT��Ϣ
// ����ֵ 	0 ���ͳɹ�		-1 ����ʧ��
int esp8266_send_mqtt_message(char *data)
{
	uint8_t time_out = 3;	// ��ʱ����
	char* p;
	//char buf[50];
	
	// ���mqttδ���� �򲻷���
	if(mqtt_connect_state == 0)
		return -1;
	
//	p = strtok(data,",");
//	strcpy(buf,p);
//	p = strtok(NULL,",");
//	while(p!=NULL)
//	{
//		strcat(buf,"\\,");
//		strcat(buf,p);
//		p = strtok(NULL,",");
//	}
	
	char cmdBuf[100];
	sprintf(cmdBuf,"AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",MQTT_TOPIC_TA,data);
	
	int result = esp8266_send_cmd(cmdBuf,"OK\r\n");
	while(result && --time_out)
	{
#ifdef SYSTEM_SUPPORT_OS	 	
		delay_us(20000);
#else		
		delay_ms(20);
#endif
		result = esp8266_send_cmd(cmdBuf,"OK\r\n");
	}
	
	
	// �������ʧ�� ����MQTT�ѶϿ�����
	if(result)
	{
		mqtt_connect_state = 0;
		return -1;
	}
	
	return 0;
}


// ���������� ��������ͨ��
// ����ֵ 	0 ���ͳɹ�		-1 ����ʧ��
int esp8266_send_heartbeat()
{
	uint8_t timeout = 4;
	const char* cmdBuf="AT+MQTTPUB=0,\"test\",\"\",0,0\r\n";
	
	int result = esp8266_send_cmd(cmdBuf,"OK\r\n");
	while(result && --timeout)
	{
#ifdef SYSTEM_SUPPORT_OS	 	
		delay_us(100000);
#else		
		delay_ms(100);
#endif
		result = esp8266_send_cmd(cmdBuf,"OK\r\n");
	}
	
	// �������ʧ�� ����MQTT�ѶϿ�����
	if(result)
	{
		mqtt_connect_state = 0;
		return -1;
	}
	
	return 0;
}


// ��ʼ��ESP8266
void esp8266_init(void)
{
	usart3_init();
}
 

// ����mqtt
void esp8266_connect_mqtt(void)
{
#ifdef SYSTEM_SUPPORT_OS	 	
	OS_ERR err;
#endif
	
	
	char temp[100]={0};
	esp8266_clear();

#if DEBUG
	printf("0. AT\r\n");
#endif
	
	while(esp8266_send_cmd("AT\r\n", "OK\r\n"))
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	
#if DEBUG
	printf("1.RST\r\n");
#endif
	
	esp8266_send_cmd("AT+RST\r\n","");
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif

#if DEBUG
	printf("2. CWMODE\r\n");
#endif
	
	while(esp8266_send_cmd("AT+CWMODE=1\r\n", "OK\r\n"))
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	
#if DEBUG
	printf("3. CWJAP\r\n");
#endif
	
	sprintf(temp,"AT+CWJAP=\"%s\",\"%s\"\r\n",ESP8266_WIFI_NAME,ESP8266_WIFI_PASSWORD);
	while(esp8266_send_cmd(temp, "GOT IP\r\n"))
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	
#if DEBUG
	printf("4. nsend USERCFG\r\n");
#endif
	
	sprintf(temp, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
	while(esp8266_send_cmd(temp, "OK\r\n"))
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	
#if DEBUG
	printf("5. nsend MQTTCONN\r\n");
#endif
	
	sprintf(temp,"AT+MQTTCONN=0,\"%s\",%d,0\r\n",ESP8266_MQTT_ADDR,ESP8266_MQTT_PORT);
	while(esp8266_send_cmd(temp, "OK\r\n"))
	{
		if(esp8266_send_cmd("AT+MQTTCONN?\r\n", ESP8266_MQTT_ADDR) == 0)
			break;
		
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	}
	
#if DEBUG
	printf("6. nsend topic\r\n");
#endif
	
	sprintf(temp,"AT+MQTTSUB=0,\"%s\",0\r\n", MQTT_TOPIC_ME);
	while(esp8266_send_cmd(temp,"OK\r\n"))
#ifdef SYSTEM_SUPPORT_OS
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
#else		
		delay_ms(500);
#endif
	
#if DEBUG
	printf("7. ESP8266 Init OK\r\n");
#endif
	
	// ���MQTT״̬Ϊ������
	mqtt_connect_state = 1;
}
 

// �ж�esp8266 && mqtt������״̬
// ����ֵ		0 ��������		-1 δ����
int esp8266_get_connected_state(void)
{
	if(mqtt_connect_state == 0)
		return -1;
	
	if(esp8266_send_cmd("AT+CWJAP?\r\n",ESP8266_WIFI_NAME) == 0)
		return -1;
	
	// ���������� ������ͨ��
	if(esp8266_send_heartbeat())
		return -1;
	
	return 0;
}
