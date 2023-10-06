#include "sys.h"

#include <string.h>

#include "includes.h"
#include "delay.h"

#include "usart1.h"
#include "usart3.h"
#include "adc1.h"
#include "iwdg.h"

#include "esp8266.h"
#include "oled.h"
#include "led.h"
#include "aht21.h"
#include "bh1750.h"
#include "ens160.h"

//开始任务
//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

///MQTT任务
#define MQTT_TASK_PRIO		4
#define MQTT_STK_SIZE 		256
OS_TCB mqtt_task_tcb;
CPU_STK MQTT_TASK_STK[MQTT_STK_SIZE];
void mqtt_task(void *p_arg);


///OLED显示任务
#define	OLED_TASK_PRIO		5
#define OLED_STK_SIZE 		128
OS_TCB oled_task_tcb;
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);


///传感器任务
#define	SENSOR_TASK_PRIO		4
#define SENSOR_STK_SIZE 		128
OS_TCB sensor_task_tcb;
CPU_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
void sensor_task(void *p_arg);

//高频发送率数据任务
#define	HEIGH_TRANS_TASK_PRIO		5
#define HEIGH_TRANS_STK_SIZE 		256
OS_TCB heigh_trans_task_tcb;
CPU_STK HEIGH_TRANS_TASK_STK[HEIGH_TRANS_STK_SIZE];
void heigh_trans_task(void *p_arg);

//低频发送数据任务
#define	LOW_TRANS_TASK_PRIO		6
#define LOW_TRANS_STK_SIZE 		128
OS_TCB low_trans_task_tcb;
CPU_STK LOW_TRANS_TASK_STK[LOW_TRANS_STK_SIZE];
void low_trans_task(void *p_arg);


// 低频传输数据信号量，用来直接启动传输高频数据
// 低频数据包括：触发器数据等
// 通过MQTT传输给用户
OS_SEM low_trans_sem;

// 高频传输数据信号量，用来直接启动传输高频数据
// 高频数据包括：传感器数据、触发器数据、触发器名称等
// 通过MQTT传输给用户
OS_SEM heigh_trans_sem;


// 触发器类型
#define		TRIGGER_TYPE_TEMP				0			// 环境温度
#define		TRIGGER_TYPE_MOIST 				1			// 环境湿度
//#define		TRIGGER_TYPE_SOIL_TEMPE			3			// 土壤温度
#define		TRIGGER_TYPE_SOIL_MOIST			2			// 土壤湿度 
#define		TRIGGER_TYPE_LIGHT				3			// 光照强度
#define		TRIGGER_TYPE_CO2				4			// 二氧化碳浓度
#define		TRIGGER_TYPE_SMOKE				5			// 烟雾
#define		TRIGGER_TYPE_FIRE				6			// 火焰

// 触发器比较方式
#define		TRIGGER_COMPARE_EQUAL			0			// 等于
#define		TRIGGER_COMPARE_GREATEREQUAL 	1			// 大于等于
#define		TRIGGER_COMPARE_LESSEQUAL 		2			// 小于等于
#define		TRIGGER_COMPARE_GREATER 		3			// 大于
#define		TRIGGER_COMPARE_LESS 			4			// 小于

// 继电器状态
#define		TRIGGER_RELAY_OPEN				1			// 闭合
#define		TRIGGER_RELAY_CLOSE				0			// 断开

// 触发器开关
#define		TRIGGER_ENABLE					1			// 开启
#define		TRIGGER_DISABLE					0			// 关闭

// 触发器数量
#define TRIGGER_LEN							50		// 最多可以设置100个触发条件

// 传感器数量
#define SENSOR_LEN							5

// 继电器数量
#define RELAY_LEN							8

// 触发器结构体
typedef struct
{
	char 			name[16];		// 触发器名称	名称大小不能超过5个字符
	int8_t			use;			// 触发器开关 开启、关闭
	uint8_t 		type;			// 触发类型 温度、湿度、光照强度等
	int64_t			value;			// 触发数值	因为有小数的原因 所以每个数据都会扩大100倍来存储
	int8_t			compare;		// 比较方式 大于、小于等
	uint8_t			relay_id;		// 触发的继电器序号
	int8_t			relay_operate;	// 触发后继电器操作 闭合、断开
} Trigger;




// 触发条件列表 索引越高优先级就越高
Trigger triggers[TRIGGER_LEN];

// mqtt接收的数据
extern char mqtt_receive_message[RX_DATA_SIZE];

// 汇总继电器引脚 方便索引
const uint16_t relay_pin_list[RELAY_LEN] = {GPIO_Pin_5,GPIO_Pin_6,GPIO_Pin_7,GPIO_Pin_8,GPIO_Pin_9,GPIO_Pin_12,GPIO_Pin_13,GPIO_Pin_14};


// 每个继电器的状态
uint8_t relay_list[RELAY_LEN] ={TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE};


// 继电器名称 名称大小不能超过5个字符
char relay_name_list[RELAY_LEN][16];

// 传感器获取的信息
uint8_t aht21_data[5] = {0};				// 记录环境温湿度信息
double light_data = 0;						// 记录光照强度
uint16_t co2_data = 0;						// 记录二氧化碳浓度
uint16_t soil_moist = 0;					// 记录土壤湿度


// 汇总传感器信息 方便索引
// 这里的数据有个特点 因为有小数的原因 所以每个数据都会扩大100倍来存储
int64_t sensor_data_list[SENSOR_LEN] = {0};


// 保存数据
// todo flash大小不够 准备外加w25q128
void save_data()
{
	// 保存触发器信息
	//flash_write((uint8_t*)triggers,FLASH_ADDRESS_VOLTAGE,sizeof(triggers));
	
	// 保存继电器名称
	//flash_write((uint8_t*)relay_name_list,FLASH_ADDRESS_VOLTAGE+(FLASH_CHECK_LEN+sizeof(triggers))*2,sizeof(relay_name_list));
}




// 更新继电器状态
void relay_update(void)
{
	for(int i=0;i<8;++i)
	{
		switch(relay_list[i])
		{			
			// 闭合继电器
			case TRIGGER_RELAY_OPEN:
				GPIO_SetBits(GPIOB,relay_pin_list[i]);
				break;
				
			// 断开继电器
			case TRIGGER_RELAY_CLOSE:
				GPIO_ResetBits(GPIOB,relay_pin_list[i]);
				break;
										
			default:
				printf("no relay operate\r\n");
		}
	}
}

// 初始化触发器系统
void trigger_init(void)
{
	int i = 0;
	
	// 初始化继电器
	GPIO_InitTypeDef   GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;				//输出模式 推挽输出
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;				//输出速率
	GPIO_InitStructure.GPIO_Pin 	= 
										GPIO_Pin_5|GPIO_Pin_6|
										GPIO_Pin_7|GPIO_Pin_8|
										GPIO_Pin_9|GPIO_Pin_12|
										GPIO_Pin_13|GPIO_Pin_14;		//引脚编号
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	// 初始化触发器
	for(i = 0;i<100;++i)
	{
		triggers[i].use = TRIGGER_DISABLE;
	}
	
	// flash大小不够 准备外加w25q128
	// 从flash中读取触发器信息
	//flash_read((uint8_t*)triggers,FLASH_ADDRESS_VOLTAGE,sizeof(triggers));
	
	// 从flash中读取继电器名称
	//flash_read((uint8_t*)relay_name_list,FLASH_ADDRESS_VOLTAGE+(FLASH_CHECK_LEN+sizeof(triggers))*2,sizeof(relay_name_list));
}


// 处理触发器操作
// 检查符合条件的触发器，并执行操作
void trigger_operation(void)
{
	int result = 0;
	int i = 0;
	
	// 记录一下要修改继电器的状态
	// 这样可以防止多个触发器操作一个继电器导致的快速开启和关闭操作
	// 这样也顺便实现了优先级的功能 下标越大的触发器 会起到最后的决定性作用

	for(i=0;i<TRIGGER_LEN;++i)
	{
		// 如果触发器被启用 则判断触发条件是否通过
		if(triggers[i].use == TRIGGER_ENABLE)
		{
			result = 0;	// 标记是否通过触发条件
			switch(triggers[i].compare)
			{
				
				//条件为小于号时
				case TRIGGER_COMPARE_LESS:
					if(sensor_data_list[triggers[i].type] < triggers[i].value)
						result = 1;
					break;
					
				//条件为大于号时
				case TRIGGER_COMPARE_GREATER:
					if(sensor_data_list[triggers[i].type] > triggers[i].value)
						result = 1;
					break;
					
				// 条件为等于号时
				case TRIGGER_COMPARE_EQUAL:
					if(sensor_data_list[triggers[i].type] == triggers[i].value)
						result = 1;
					break;
				
					//条件为小于等于号时
				case TRIGGER_COMPARE_LESSEQUAL:
					if(sensor_data_list[triggers[i].type] <= triggers[i].value)
						result = 1;
					break;
					
				//条件为大于等于号时
				case TRIGGER_COMPARE_GREATEREQUAL:
					if(sensor_data_list[triggers[i].type] >= triggers[i].value)
						result = 1;
					break;
					
				default:
					printf("no compare\r\n");
					
			}
			
			// 如果触发了条件
			if(result)
			{
				relay_list[triggers[i].relay_id] = triggers[i].relay_operate;
			}
			
		}
	}
	
	
	// 更新继电器状态
	relay_update();
}







// 将一个触发器的内容发送出去
void send_trigger_data(uint8_t id)
{
	char format_data[128];
	char temp[20];
	
	//指令格式： 优先级id 名称 开关 类型 触发的值 比较方式 继电器id 继电器操作方式
	strcpy(format_data,"#trigger_data#");
	
	sprintf(temp," %d",id);
	strcat(format_data,temp);
	
	strcat(format_data," ");
	strcat(format_data,triggers[id].name);
	
	if(triggers[id].use == TRIGGER_ENABLE)
		strcat(format_data," 1");
	else
		strcat(format_data," 0");
	
	sprintf(temp," %d",triggers[id].type);
	strcat(format_data,temp);
	
	sprintf(temp," %lld",triggers[id].value);
	strcat(format_data,temp);
	
	sprintf(temp," %d",triggers[id].compare);
	strcat(format_data,temp);
	
	sprintf(temp," %d",triggers[id].relay_id);
	strcat(format_data,temp);
	
	sprintf(temp," %d",triggers[id].relay_operate);
	strcat(format_data,temp);
	esp8266_send_mqtt_message(format_data);
}





// 主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	SystemInit();	//由于本工程是由stm32f103zet6移植所来，因此这里因加入本函数确保delay函数正常使用
	
	delay_init();  			// 初始化时钟
	usart1_init();			// 初始化串口1
	oled_init();			// 初始化OLED
	led_init();				// 初始化LED
	trigger_init();			// 初始化触发器
	adc1_init();			// 初始化模数转换器1
	aht21_init();			// 初始化温湿度传感器
	bh1750_init();			// 初始化光照传感器
	ENS160_Init();			// 初始化空气质量传感器 	todo:获取到的eCO2不适合大棚环境
	esp8266_init();			// 初始化ESP8266
	iwdg_init(25);			// 初始化独立看门狗
	
#if DEBUG
	printf("程序启动\r\n");
#endif
	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}


// 开始任务任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//进入临界区
	
	//创建”高频传输数据“信号量
    OSSemCreate(&heigh_trans_sem,"heigh_trans_sem",1,&err);

	//创建“低频传输数据”信号量
    OSSemCreate(&low_trans_sem,"low_trans_sem",1,&err);

	
	
	//创建MQTT任务
	OSTaskCreate((OS_TCB 	* )&mqtt_task_tcb,		
				 (CPU_CHAR	* )"MQTT task", 		
                 (OS_TASK_PTR )mqtt_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )MQTT_TASK_PRIO,     
                 (CPU_STK   * )&MQTT_TASK_STK[0],	
                 (CPU_STK_SIZE)MQTT_STK_SIZE/10,	
                 (CPU_STK_SIZE)MQTT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建OLED显示任务
	OSTaskCreate((OS_TCB 	* )&oled_task_tcb,		
				 (CPU_CHAR	* )"OLED task", 		
                 (OS_TASK_PTR )oled_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )OLED_TASK_PRIO,     	
                 (CPU_STK   * )&OLED_TASK_STK[0],	
                 (CPU_STK_SIZE)OLED_STK_SIZE/10,	
                 (CPU_STK_SIZE)OLED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建传感器任务
	OSTaskCreate((OS_TCB 	* )&sensor_task_tcb,		
				 (CPU_CHAR	* )"sensor task", 		
                 (OS_TASK_PTR )sensor_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )SENSOR_TASK_PRIO,     	
                 (CPU_STK   * )&SENSOR_TASK_STK[0],	
                 (CPU_STK_SIZE)SENSOR_STK_SIZE/10,	
                 (CPU_STK_SIZE)SENSOR_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
				 
	//低频发送数据函数
	OSTaskCreate((OS_TCB 	* )&low_trans_task_tcb,		
				 (CPU_CHAR	* )"low trans task", 		
                 (OS_TASK_PTR )low_trans_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LOW_TRANS_TASK_PRIO,     	
                 (CPU_STK   * )&LOW_TRANS_TASK_STK[0],	
                 (CPU_STK_SIZE)LOW_TRANS_STK_SIZE/10,	
                 (CPU_STK_SIZE)LOW_TRANS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
	
	//高频发送数据函数
	OSTaskCreate((OS_TCB 	* )&heigh_trans_task_tcb,		
				 (CPU_CHAR	* )"heigh trans task", 		
                 (OS_TASK_PTR )heigh_trans_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )HEIGH_TRANS_TASK_PRIO,     	
                 (CPU_STK   * )&HEIGH_TRANS_TASK_STK[0],	
                 (CPU_STK_SIZE)HEIGH_TRANS_STK_SIZE/10,	
                 (CPU_STK_SIZE)HEIGH_TRANS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}


// MQTT任务函数
void mqtt_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	esp8266_connect_mqtt();		// 连接网络
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		// 判断MQTT连接状态 如果不在线则重连
		if(esp8266_get_connected_state() == -1)
		{
#if DEBUG
			printf("MQTT 已断开连接\r\n");
#endif
			led_set_green(0);
			esp8266_connect_mqtt();
		}
		else
		{
			led_set_green(1);
		}
		
		OS_CRITICAL_ENTER();
#if DEBUG
		printf("mqtt runing...\r\n");
#endif
		
		// 喂狗
		iwdg_feed();
		
		if(mqtt_receive_message[0] != '\0')
		{
			char temp_message[RX_DATA_SIZE];
			strcpy(temp_message,mqtt_receive_message);
			mqtt_receive_message[0] = '\0';
			
			// 提取指令
			char *p = strtok(temp_message,"#");
			if(strcmp(p,"connect") == 0)
			{
				esp8266_send_mqtt_message("ok");
			}
			else if(strcmp(p,"set_relay_name") == 0)
			{
				unsigned int id;
				char name[16];
				p = strtok(NULL,"#");
				sscanf(p,"%u %s",&id,name);
				strcpy(relay_name_list[id],name);
				esp8266_send_mqtt_message("ok");
				
				// 保存数据
				save_data();
				
				// 释放信号量 来发送更新的数据
				OSSemPost(&heigh_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"get_trigger_data") == 0)
			{
				esp8266_send_mqtt_message("ok");
				
				// 释放信号量 来发送更新的数据
				OSSemPost(&low_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"set_relay_open") == 0)
			{
				unsigned int id;
				p = strtok(NULL,"#");
				sscanf(p,"%u",&id);
				relay_list[id] = TRIGGER_RELAY_OPEN;
				
				esp8266_send_mqtt_message("ok");
				
				// 更新继电器状态
				relay_update();
				
				// 释放信号量 来发送更新的数据
				OSSemPost(&heigh_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"set_relay_close") == 0)
			{
				unsigned int id;
				p = strtok(NULL,"#");
				sscanf(p,"%u",&id);
				relay_list[id] = TRIGGER_RELAY_CLOSE;
				
				esp8266_send_mqtt_message("ok");
				
				// 更新继电器状态
				relay_update();
				
				// 释放信号量 来发送更新的数据
				OSSemPost(&heigh_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"set_trigger") == 0)
			{
				unsigned int id;
				char name[16];
				int use;
				unsigned int type;
				int64_t value;
				int compare;
				unsigned int relay_id;
				int relay_operate;
				
				p = strtok(NULL,"#");
				sscanf(p,"%u %s %d %u %lld %d %u %d",&id,name,&use,&type,&value,&compare,&relay_id,&relay_operate);
				strcpy(triggers[id].name,name);
				triggers[id].use = use;
				triggers[id].type = type;
				triggers[id].value = value;
				triggers[id].compare = compare;
				triggers[id].relay_id = relay_id;
				triggers[id].relay_operate = relay_operate;
				
				esp8266_send_mqtt_message("ok");
				
				// 发送更新的一个触发器数据
				send_trigger_data(id);

				// 保存数据
				save_data();
			}
		}
		
		
		// 喂狗
		iwdg_feed();
		

		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}



// OLED显示任务函数
void oled_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	char temp_format[20];		// 字符串格式化临时数据
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
		
#if DEBUG
		printf("oled runing...\r\n");
#endif
		oled_clear();
		
		oled_show_chinese(0,0,0);	//环
		oled_show_chinese(16,0,1);	//境
		oled_show_chinese(32,0,6);	//温
		oled_show_chinese(48,0,10);	//度
		sprintf(temp_format,"%d.%d",aht21_data[2],aht21_data[3]);
		oled_show_string(80,0,temp_format,16);
		
		
		oled_show_chinese(0,2,0);	//环
		oled_show_chinese(16,2,1);	//境
		oled_show_chinese(32,2,7);	//湿
		oled_show_chinese(48,2,10);	//度
		sprintf(temp_format,"%d.%d",aht21_data[0],aht21_data[1]);
		oled_show_string(80,2,temp_format,16);
		
		OS_CRITICAL_EXIT();
		
		
		
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
		
		
	
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
		
		oled_clear();

		oled_show_chinese(0,0,4);	//光
		oled_show_chinese(16,0,5);	//照
		oled_show_chinese(32,0,9);	//强
		oled_show_chinese(48,0,10);	//度
		sprintf(temp_format,"%d.%d",(uint16_t)light_data,(uint16_t)(light_data*100)%100);
		if(strlen(temp_format)<5)
			oled_show_string(80,0,temp_format,16);
		else
			oled_show_string(65,0,temp_format,16);
			
		oled_show_string(0,2,"CO",16);
		oled_show_string(16,3,"2",12);
		oled_show_chinese(32,2,8);	//浓
		oled_show_chinese(48,2,10);	//度
		sprintf(temp_format,"%d",co2_data);
		oled_show_string(80,2,temp_format,16);
		
		oled_show_chinese(0,4,2);	//土
		oled_show_chinese(16,4,3);	//壤
		oled_show_chinese(32,4,7);	//湿
		oled_show_chinese(48,4,10);	//度
		sprintf(temp_format,"%d%%",soil_moist);
		oled_show_string(80,4,temp_format,16);
		
		
		OS_CRITICAL_EXIT();

		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


// 传感器任务函数
void sensor_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("sensor runing...\r\n");
#endif
		
		// 获取环境温度湿度
		aht21_read_data(aht21_data);
		sensor_data_list[TRIGGER_TYPE_TEMP] =  aht21_data[2]*100+aht21_data[3];			// 汇总温度数据
		sensor_data_list[TRIGGER_TYPE_MOIST] =  aht21_data[0]*100+aht21_data[1];		// 汇总湿度数据
		
		// todo: 模拟的数据 精度不高
		// 获取土壤湿度
		soil_moist = adc_get_soil_moist();
		sensor_data_list[TRIGGER_TYPE_SOIL_MOIST] =  soil_moist*100;			// 汇总土壤湿度数据
		
		
		// 获取光照强度
		light_data = bh1750_read_data();
		sensor_data_list[TRIGGER_TYPE_LIGHT] =  light_data*100;			// 汇总光照强度数据
		
		// 获取二氧化碳浓度
		ENS160_Get_DATA_ECO2(&co2_data);
		sensor_data_list[TRIGGER_TYPE_CO2] =  co2_data*100;				// 汇总二氧化碳浓度数据
		
		// 检查符合条件的触发器，并执行操作
		trigger_operation();
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,2,500,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

// 低频发送数据任务函数
void low_trans_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OSSemPend( &low_trans_sem,
					1000,                  	//200hz 大约等待5秒
					OS_OPT_PEND_BLOCKING,   //进行阻塞
					0,                      //不记录时间戳
					&err                    //出错处理
				  );
		OSTimeDly(400,OS_OPT_TIME_DLY,&err);
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("low trans runing...\r\n");
#endif
		
		for(int i=0;i<TRIGGER_LEN;++i)
		{
			// 喂狗
			iwdg_feed();
			
			send_trigger_data(i);
		}
		
		OS_CRITICAL_EXIT();
	}
}

// 高频发送数据任务函数
void heigh_trans_task(void *p_arg)
{
	char temp[20];
	char format_data[128];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OSSemPend( &heigh_trans_sem,
					400,                   	//200hz 大约等待2秒
					OS_OPT_PEND_BLOCKING,   //进行阻塞
					0,                      //不记录时间戳
					&err                    //出错处理
				  );
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("heigh trans runing...\r\n");
#endif
		// 将传感器数据发送出去
		strcpy(format_data,"#sensor_data#");
		for(int i=0;i<SENSOR_LEN;++i)
		{
			sprintf(temp," %lld",sensor_data_list[i]);
			strcat(format_data,temp);
		}
		esp8266_send_mqtt_message(format_data);
		
		// 喂狗
		iwdg_feed();
		
		
		// 将继电器名称发送出去
		strcpy(format_data,"#relay_name#");
		for(int i=0;i<RELAY_LEN;++i)
		{
			strcat(format_data," ");
			strcat(format_data,relay_name_list[i]);
		}
		esp8266_send_mqtt_message(format_data);
		
		// 喂狗
		iwdg_feed();
		
		
		// 将继电器数据发送出去
		strcpy(format_data,"#relay_data#");
		for(int i=0;i<RELAY_LEN;++i)
		{
			if(relay_list[i] == TRIGGER_RELAY_OPEN)
				strcat(format_data," 1");
			else
				strcat(format_data," 0");
		}
		esp8266_send_mqtt_message(format_data);
		
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_EXIT();
	}
}
