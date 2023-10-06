#ifndef __ESP8266_H
#define __ESP8266_H	


#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志


// 清空缓存
void esp8266_clear(void);
 
// 发送MQTT消息
// 返回值 	0 发送成功		-1 发送失败
int esp8266_send_mqtt_message(char *data);

// 初始化ESP8266
void esp8266_init(void);

// 连接mqtt
void esp8266_connect_mqtt(void);

// 判断esp8266 && mqtt的连接状态
// 返回值		0 连接正常		-1 未连接
int esp8266_get_connected_state(void);

#endif
