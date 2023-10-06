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
#define MQTT_USERNAME  	""				//名字
#define MQTT_PASSWORD  	""				//密码

#define MQTT_TOPIC_ME	"smart_agriculture_client"
#define MQTT_TOPIC_TA	"smart_agriculture_ui"
 
extern char esp8266_buf[RX_DATA_SIZE];
extern uint8_t esp8266_cnt;
uint8_t esp8266_cntPre = 0;

//mqtt	连接状态 0：未连接 1：连接
extern uint8_t mqtt_connect_state;

//mqtt 接受的消息
extern char mqtt_receive_message[RX_DATA_SIZE];


// 清空缓存
void esp8266_clear(void)
{
 
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
 
}
 
// 判断消息是否接受完成	需要循环调用
int esp8266_wait_receive(void)
{
 
	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志
 
}
 
// 发送命令
// 返回值	0 核对正确	-1 核对有误
int esp8266_send_cmd(const char *cmd, char *res)
{
	
	uint8_t time_out = 200;
 
	usart3_send_string(cmd);
	
	while(time_out--)
	{
		if(esp8266_wait_receive() == REV_OK)							//如果收到数据
		{
			
			//printf("%s\r\n",esp8266_buf); 
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
#if DEBUG
				if(strstr(cmd,"MQTTSUB") != NULL)
					printf("%s\r\n",esp8266_buf); 
#endif
				esp8266_clear();									//清空缓存
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
 
 
// 发送MQTT消息
// 返回值 	0 发送成功		-1 发送失败
int esp8266_send_mqtt_message(char *data)
{
	uint8_t time_out = 3;	// 超时次数
	char* p;
	//char buf[50];
	
	// 如果mqtt未连接 则不发送
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
	
	
	// 如果发送失败 则标记MQTT已断开连接
	if(result)
	{
		mqtt_connect_state = 0;
		return -1;
	}
	
	return 0;
}


// 发送心跳包 来测试连通性
// 返回值 	0 发送成功		-1 发送失败
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
	
	// 如果发送失败 则标记MQTT已断开连接
	if(result)
	{
		mqtt_connect_state = 0;
		return -1;
	}
	
	return 0;
}


// 初始化ESP8266
void esp8266_init(void)
{
	usart3_init();
}
 

// 连接mqtt
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
	
	// 标记MQTT状态为已连接
	mqtt_connect_state = 1;
}
 

// 判断esp8266 && mqtt的连接状态
// 返回值		0 连接正常		-1 未连接
int esp8266_get_connected_state(void)
{
	if(mqtt_connect_state == 0)
		return -1;
	
	if(esp8266_send_cmd("AT+CWJAP?\r\n",ESP8266_WIFI_NAME) == 0)
		return -1;
	
	// 发送心跳包 测试连通性
	if(esp8266_send_heartbeat())
		return -1;
	
	return 0;
}
