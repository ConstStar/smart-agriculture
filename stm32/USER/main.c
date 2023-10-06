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

//��ʼ����
//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

///MQTT����
#define MQTT_TASK_PRIO		4
#define MQTT_STK_SIZE 		256
OS_TCB mqtt_task_tcb;
CPU_STK MQTT_TASK_STK[MQTT_STK_SIZE];
void mqtt_task(void *p_arg);


///OLED��ʾ����
#define	OLED_TASK_PRIO		5
#define OLED_STK_SIZE 		128
OS_TCB oled_task_tcb;
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);


///����������
#define	SENSOR_TASK_PRIO		4
#define SENSOR_STK_SIZE 		128
OS_TCB sensor_task_tcb;
CPU_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
void sensor_task(void *p_arg);

//��Ƶ��������������
#define	HEIGH_TRANS_TASK_PRIO		5
#define HEIGH_TRANS_STK_SIZE 		256
OS_TCB heigh_trans_task_tcb;
CPU_STK HEIGH_TRANS_TASK_STK[HEIGH_TRANS_STK_SIZE];
void heigh_trans_task(void *p_arg);

//��Ƶ������������
#define	LOW_TRANS_TASK_PRIO		6
#define LOW_TRANS_STK_SIZE 		128
OS_TCB low_trans_task_tcb;
CPU_STK LOW_TRANS_TASK_STK[LOW_TRANS_STK_SIZE];
void low_trans_task(void *p_arg);


// ��Ƶ���������ź���������ֱ�����������Ƶ����
// ��Ƶ���ݰ��������������ݵ�
// ͨ��MQTT������û�
OS_SEM low_trans_sem;

// ��Ƶ���������ź���������ֱ�����������Ƶ����
// ��Ƶ���ݰ��������������ݡ����������ݡ����������Ƶ�
// ͨ��MQTT������û�
OS_SEM heigh_trans_sem;


// ����������
#define		TRIGGER_TYPE_TEMP				0			// �����¶�
#define		TRIGGER_TYPE_MOIST 				1			// ����ʪ��
//#define		TRIGGER_TYPE_SOIL_TEMPE			3			// �����¶�
#define		TRIGGER_TYPE_SOIL_MOIST			2			// ����ʪ�� 
#define		TRIGGER_TYPE_LIGHT				3			// ����ǿ��
#define		TRIGGER_TYPE_CO2				4			// ������̼Ũ��
#define		TRIGGER_TYPE_SMOKE				5			// ����
#define		TRIGGER_TYPE_FIRE				6			// ����

// �������ȽϷ�ʽ
#define		TRIGGER_COMPARE_EQUAL			0			// ����
#define		TRIGGER_COMPARE_GREATEREQUAL 	1			// ���ڵ���
#define		TRIGGER_COMPARE_LESSEQUAL 		2			// С�ڵ���
#define		TRIGGER_COMPARE_GREATER 		3			// ����
#define		TRIGGER_COMPARE_LESS 			4			// С��

// �̵���״̬
#define		TRIGGER_RELAY_OPEN				1			// �պ�
#define		TRIGGER_RELAY_CLOSE				0			// �Ͽ�

// ����������
#define		TRIGGER_ENABLE					1			// ����
#define		TRIGGER_DISABLE					0			// �ر�

// ����������
#define TRIGGER_LEN							50		// ����������100����������

// ����������
#define SENSOR_LEN							5

// �̵�������
#define RELAY_LEN							8

// �������ṹ��
typedef struct
{
	char 			name[16];		// ����������	���ƴ�С���ܳ���5���ַ�
	int8_t			use;			// ���������� �������ر�
	uint8_t 		type;			// �������� �¶ȡ�ʪ�ȡ�����ǿ�ȵ�
	int64_t			value;			// ������ֵ	��Ϊ��С����ԭ�� ����ÿ�����ݶ�������100�����洢
	int8_t			compare;		// �ȽϷ�ʽ ���ڡ�С�ڵ�
	uint8_t			relay_id;		// �����ļ̵������
	int8_t			relay_operate;	// ������̵������� �պϡ��Ͽ�
} Trigger;




// ���������б� ����Խ�����ȼ���Խ��
Trigger triggers[TRIGGER_LEN];

// mqtt���յ�����
extern char mqtt_receive_message[RX_DATA_SIZE];

// ���̵ܼ������� ��������
const uint16_t relay_pin_list[RELAY_LEN] = {GPIO_Pin_5,GPIO_Pin_6,GPIO_Pin_7,GPIO_Pin_8,GPIO_Pin_9,GPIO_Pin_12,GPIO_Pin_13,GPIO_Pin_14};


// ÿ���̵�����״̬
uint8_t relay_list[RELAY_LEN] ={TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE,
								TRIGGER_RELAY_CLOSE,TRIGGER_RELAY_CLOSE};


// �̵������� ���ƴ�С���ܳ���5���ַ�
char relay_name_list[RELAY_LEN][16];

// ��������ȡ����Ϣ
uint8_t aht21_data[5] = {0};				// ��¼������ʪ����Ϣ
double light_data = 0;						// ��¼����ǿ��
uint16_t co2_data = 0;						// ��¼������̼Ũ��
uint16_t soil_moist = 0;					// ��¼����ʪ��


// ���ܴ�������Ϣ ��������
// ����������и��ص� ��Ϊ��С����ԭ�� ����ÿ�����ݶ�������100�����洢
int64_t sensor_data_list[SENSOR_LEN] = {0};


// ��������
// todo flash��С���� ׼�����w25q128
void save_data()
{
	// ���津������Ϣ
	//flash_write((uint8_t*)triggers,FLASH_ADDRESS_VOLTAGE,sizeof(triggers));
	
	// ����̵�������
	//flash_write((uint8_t*)relay_name_list,FLASH_ADDRESS_VOLTAGE+(FLASH_CHECK_LEN+sizeof(triggers))*2,sizeof(relay_name_list));
}




// ���¼̵���״̬
void relay_update(void)
{
	for(int i=0;i<8;++i)
	{
		switch(relay_list[i])
		{			
			// �պϼ̵���
			case TRIGGER_RELAY_OPEN:
				GPIO_SetBits(GPIOB,relay_pin_list[i]);
				break;
				
			// �Ͽ��̵���
			case TRIGGER_RELAY_CLOSE:
				GPIO_ResetBits(GPIOB,relay_pin_list[i]);
				break;
										
			default:
				printf("no relay operate\r\n");
		}
	}
}

// ��ʼ��������ϵͳ
void trigger_init(void)
{
	int i = 0;
	
	// ��ʼ���̵���
	GPIO_InitTypeDef   GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;				//���ģʽ �������
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;				//�������
	GPIO_InitStructure.GPIO_Pin 	= 
										GPIO_Pin_5|GPIO_Pin_6|
										GPIO_Pin_7|GPIO_Pin_8|
										GPIO_Pin_9|GPIO_Pin_12|
										GPIO_Pin_13|GPIO_Pin_14;		//���ű��
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	// ��ʼ��������
	for(i = 0;i<100;++i)
	{
		triggers[i].use = TRIGGER_DISABLE;
	}
	
	// flash��С���� ׼�����w25q128
	// ��flash�ж�ȡ��������Ϣ
	//flash_read((uint8_t*)triggers,FLASH_ADDRESS_VOLTAGE,sizeof(triggers));
	
	// ��flash�ж�ȡ�̵�������
	//flash_read((uint8_t*)relay_name_list,FLASH_ADDRESS_VOLTAGE+(FLASH_CHECK_LEN+sizeof(triggers))*2,sizeof(relay_name_list));
}


// ������������
// �����������Ĵ���������ִ�в���
void trigger_operation(void)
{
	int result = 0;
	int i = 0;
	
	// ��¼һ��Ҫ�޸ļ̵�����״̬
	// �������Է�ֹ�������������һ���̵������µĿ��ٿ����͹رղ���
	// ����Ҳ˳��ʵ�������ȼ��Ĺ��� �±�Խ��Ĵ����� �������ľ���������

	for(i=0;i<TRIGGER_LEN;++i)
	{
		// ��������������� ���жϴ��������Ƿ�ͨ��
		if(triggers[i].use == TRIGGER_ENABLE)
		{
			result = 0;	// ����Ƿ�ͨ����������
			switch(triggers[i].compare)
			{
				
				//����ΪС�ں�ʱ
				case TRIGGER_COMPARE_LESS:
					if(sensor_data_list[triggers[i].type] < triggers[i].value)
						result = 1;
					break;
					
				//����Ϊ���ں�ʱ
				case TRIGGER_COMPARE_GREATER:
					if(sensor_data_list[triggers[i].type] > triggers[i].value)
						result = 1;
					break;
					
				// ����Ϊ���ں�ʱ
				case TRIGGER_COMPARE_EQUAL:
					if(sensor_data_list[triggers[i].type] == triggers[i].value)
						result = 1;
					break;
				
					//����ΪС�ڵ��ں�ʱ
				case TRIGGER_COMPARE_LESSEQUAL:
					if(sensor_data_list[triggers[i].type] <= triggers[i].value)
						result = 1;
					break;
					
				//����Ϊ���ڵ��ں�ʱ
				case TRIGGER_COMPARE_GREATEREQUAL:
					if(sensor_data_list[triggers[i].type] >= triggers[i].value)
						result = 1;
					break;
					
				default:
					printf("no compare\r\n");
					
			}
			
			// �������������
			if(result)
			{
				relay_list[triggers[i].relay_id] = triggers[i].relay_operate;
			}
			
		}
	}
	
	
	// ���¼̵���״̬
	relay_update();
}







// ��һ�������������ݷ��ͳ�ȥ
void send_trigger_data(uint8_t id)
{
	char format_data[128];
	char temp[20];
	
	//ָ���ʽ�� ���ȼ�id ���� ���� ���� ������ֵ �ȽϷ�ʽ �̵���id �̵���������ʽ
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





// ������
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	SystemInit();	//���ڱ���������stm32f103zet6��ֲ�����������������뱾����ȷ��delay��������ʹ��
	
	delay_init();  			// ��ʼ��ʱ��
	usart1_init();			// ��ʼ������1
	oled_init();			// ��ʼ��OLED
	led_init();				// ��ʼ��LED
	trigger_init();			// ��ʼ��������
	adc1_init();			// ��ʼ��ģ��ת����1
	aht21_init();			// ��ʼ����ʪ�ȴ�����
	bh1750_init();			// ��ʼ�����մ�����
	ENS160_Init();			// ��ʼ���������������� 	todo:��ȡ����eCO2���ʺϴ��ﻷ��
	esp8266_init();			// ��ʼ��ESP8266
	iwdg_init(25);			// ��ʼ���������Ź�
	
#if DEBUG
	printf("��������\r\n");
#endif
	
	OSInit(&err);		    //��ʼ��UCOSIII
	OS_CRITICAL_ENTER();	//�����ٽ���			 
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);      //����UCOSIII
}


// ��ʼ����������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	
	//��������Ƶ�������ݡ��ź���
    OSSemCreate(&heigh_trans_sem,"heigh_trans_sem",1,&err);

	//��������Ƶ�������ݡ��ź���
    OSSemCreate(&low_trans_sem,"low_trans_sem",1,&err);

	
	
	//����MQTT����
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
				 
	//����OLED��ʾ����
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
				 
	//��������������
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
				 
				 
	//��Ƶ�������ݺ���
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
	
	//��Ƶ�������ݺ���
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
				 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}


// MQTT������
void mqtt_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	esp8266_connect_mqtt();		// ��������
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
		// �ж�MQTT����״̬ ���������������
		if(esp8266_get_connected_state() == -1)
		{
#if DEBUG
			printf("MQTT �ѶϿ�����\r\n");
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
		
		// ι��
		iwdg_feed();
		
		if(mqtt_receive_message[0] != '\0')
		{
			char temp_message[RX_DATA_SIZE];
			strcpy(temp_message,mqtt_receive_message);
			mqtt_receive_message[0] = '\0';
			
			// ��ȡָ��
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
				
				// ��������
				save_data();
				
				// �ͷ��ź��� �����͸��µ�����
				OSSemPost(&heigh_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"get_trigger_data") == 0)
			{
				esp8266_send_mqtt_message("ok");
				
				// �ͷ��ź��� �����͸��µ�����
				OSSemPost(&low_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"set_relay_open") == 0)
			{
				unsigned int id;
				p = strtok(NULL,"#");
				sscanf(p,"%u",&id);
				relay_list[id] = TRIGGER_RELAY_OPEN;
				
				esp8266_send_mqtt_message("ok");
				
				// ���¼̵���״̬
				relay_update();
				
				// �ͷ��ź��� �����͸��µ�����
				OSSemPost(&heigh_trans_sem,OS_OPT_POST_1,&err);
			}
			else if(strcmp(p,"set_relay_close") == 0)
			{
				unsigned int id;
				p = strtok(NULL,"#");
				sscanf(p,"%u",&id);
				relay_list[id] = TRIGGER_RELAY_CLOSE;
				
				esp8266_send_mqtt_message("ok");
				
				// ���¼̵���״̬
				relay_update();
				
				// �ͷ��ź��� �����͸��µ�����
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
				
				// ���͸��µ�һ������������
				send_trigger_data(id);

				// ��������
				save_data();
			}
		}
		
		
		// ι��
		iwdg_feed();
		

		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}



// OLED��ʾ������
void oled_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	char temp_format[20];		// �ַ�����ʽ����ʱ����
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
		
#if DEBUG
		printf("oled runing...\r\n");
#endif
		oled_clear();
		
		oled_show_chinese(0,0,0);	//��
		oled_show_chinese(16,0,1);	//��
		oled_show_chinese(32,0,6);	//��
		oled_show_chinese(48,0,10);	//��
		sprintf(temp_format,"%d.%d",aht21_data[2],aht21_data[3]);
		oled_show_string(80,0,temp_format,16);
		
		
		oled_show_chinese(0,2,0);	//��
		oled_show_chinese(16,2,1);	//��
		oled_show_chinese(32,2,7);	//ʪ
		oled_show_chinese(48,2,10);	//��
		sprintf(temp_format,"%d.%d",aht21_data[0],aht21_data[1]);
		oled_show_string(80,2,temp_format,16);
		
		OS_CRITICAL_EXIT();
		
		
		
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
		
		
	
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
		
		oled_clear();

		oled_show_chinese(0,0,4);	//��
		oled_show_chinese(16,0,5);	//��
		oled_show_chinese(32,0,9);	//ǿ
		oled_show_chinese(48,0,10);	//��
		sprintf(temp_format,"%d.%d",(uint16_t)light_data,(uint16_t)(light_data*100)%100);
		if(strlen(temp_format)<5)
			oled_show_string(80,0,temp_format,16);
		else
			oled_show_string(65,0,temp_format,16);
			
		oled_show_string(0,2,"CO",16);
		oled_show_string(16,3,"2",12);
		oled_show_chinese(32,2,8);	//Ũ
		oled_show_chinese(48,2,10);	//��
		sprintf(temp_format,"%d",co2_data);
		oled_show_string(80,2,temp_format,16);
		
		oled_show_chinese(0,4,2);	//��
		oled_show_chinese(16,4,3);	//��
		oled_show_chinese(32,4,7);	//ʪ
		oled_show_chinese(48,4,10);	//��
		sprintf(temp_format,"%d%%",soil_moist);
		oled_show_string(80,4,temp_format,16);
		
		
		OS_CRITICAL_EXIT();

		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


// ������������
void sensor_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("sensor runing...\r\n");
#endif
		
		// ��ȡ�����¶�ʪ��
		aht21_read_data(aht21_data);
		sensor_data_list[TRIGGER_TYPE_TEMP] =  aht21_data[2]*100+aht21_data[3];			// �����¶�����
		sensor_data_list[TRIGGER_TYPE_MOIST] =  aht21_data[0]*100+aht21_data[1];		// ����ʪ������
		
		// todo: ģ������� ���Ȳ���
		// ��ȡ����ʪ��
		soil_moist = adc_get_soil_moist();
		sensor_data_list[TRIGGER_TYPE_SOIL_MOIST] =  soil_moist*100;			// ��������ʪ������
		
		
		// ��ȡ����ǿ��
		light_data = bh1750_read_data();
		sensor_data_list[TRIGGER_TYPE_LIGHT] =  light_data*100;			// ���ܹ���ǿ������
		
		// ��ȡ������̼Ũ��
		ENS160_Get_DATA_ECO2(&co2_data);
		sensor_data_list[TRIGGER_TYPE_CO2] =  co2_data*100;				// ���ܶ�����̼Ũ������
		
		// �����������Ĵ���������ִ�в���
		trigger_operation();
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,2,500,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

// ��Ƶ��������������
void low_trans_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
		OSSemPend( &low_trans_sem,
					1000,                  	//200hz ��Լ�ȴ�5��
					OS_OPT_PEND_BLOCKING,   //��������
					0,                      //����¼ʱ���
					&err                    //������
				  );
		OSTimeDly(400,OS_OPT_TIME_DLY,&err);
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("low trans runing...\r\n");
#endif
		
		for(int i=0;i<TRIGGER_LEN;++i)
		{
			// ι��
			iwdg_feed();
			
			send_trigger_data(i);
		}
		
		OS_CRITICAL_EXIT();
	}
}

// ��Ƶ��������������
void heigh_trans_task(void *p_arg)
{
	char temp[20];
	char format_data[128];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
		OSSemPend( &heigh_trans_sem,
					400,                   	//200hz ��Լ�ȴ�2��
					OS_OPT_PEND_BLOCKING,   //��������
					0,                      //����¼ʱ���
					&err                    //������
				  );
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("heigh trans runing...\r\n");
#endif
		// �����������ݷ��ͳ�ȥ
		strcpy(format_data,"#sensor_data#");
		for(int i=0;i<SENSOR_LEN;++i)
		{
			sprintf(temp," %lld",sensor_data_list[i]);
			strcat(format_data,temp);
		}
		esp8266_send_mqtt_message(format_data);
		
		// ι��
		iwdg_feed();
		
		
		// ���̵������Ʒ��ͳ�ȥ
		strcpy(format_data,"#relay_name#");
		for(int i=0;i<RELAY_LEN;++i)
		{
			strcat(format_data," ");
			strcat(format_data,relay_name_list[i]);
		}
		esp8266_send_mqtt_message(format_data);
		
		// ι��
		iwdg_feed();
		
		
		// ���̵������ݷ��ͳ�ȥ
		strcpy(format_data,"#relay_data#");
		for(int i=0;i<RELAY_LEN;++i)
		{
			if(relay_list[i] == TRIGGER_RELAY_OPEN)
				strcat(format_data," 1");
			else
				strcat(format_data," 0");
		}
		esp8266_send_mqtt_message(format_data);
		
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_EXIT();
	}
}
