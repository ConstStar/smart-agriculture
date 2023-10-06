#include "sys.h"

#include "stm32f10x.h"
#include "stm32f10x_adc.h"


#include "adc1.h"

void adc1_init(void)
{ 
    GPIO_InitTypeDef     GPIO_InitStructure;//��ʼ���ɼ�ADC�Ľ�
    ADC_InitTypeDef     ADC_InitStructure;//����ADC�ṹ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//����ADCʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//����GPIOAʱ��
    
    //��Ҫ��������ADCCLKʱ��Ԥ��Ƶ
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//����ADCʱ��Ԥ��Ƶ ADCCLK=72Mhz/6=12Mhz
    
    //����ģ����������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//���ó�ģ������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //ѡ������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;// ������������
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    //���ù�����
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_55Cycles5);
    
    //����ADC�ṹ���ʼ��
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//����ADCģʽ��˫ͨ��ģʽ���ǵ�ͨ��ģʽ
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//���ݶ���ģʽ �����
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None ;  //�ⲿ����ת��ѡ����Ǵ������ƵĴ���Դ  �� ʹ���ڲ�������������� ��
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //����ת��ģʽ������ѡ��������ת�����ǵ���ת��  DISABLE����ģʽ  ENABLE����ģʽ
    ADC_InitStructure.ADC_NbrOfChannel = 1; //ͨ����Ŀ����ָ��ɨ��ģʽ�£��ܹ����õ�����ͨ��
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; //ɨ��ת��ģʽ������ѡ����ɨ��ģʽ���Ƿ�ɨ��ģʽ        DISABLE��ɨ��ģʽ��   ENABLEɨ��ģʽ
    ADC_Init(ADC1,&ADC_InitStructure);
    
    //����ADC��Դ
    ADC_Cmd(ADC1,ENABLE);
    
    //��ʼADCУ׼
    ADC_ResetCalibration(ADC1); //��ʼ��λУ׼ ����־λ��1Ӳ������
    while(ADC_GetCalibrationStatus(ADC1)==SET); //�ж�У׼��־λ���Ƿ�Ϊ ��Ϊ0У׼���
    ADC_StartCalibration(ADC1);    //��ʼУ׼
    while(ADC_GetCalibrationStatus(ADC1)==SET);//�ж��Ƿ����У׼
}

// ��ȡ����ʪ�Ȱٷֱȹ�ֵ ����֤ʮ��׼ȷ
uint8_t adc_get_soil_moist(void)
{
	uint16_t data;
    ADC_SoftwareStartConvCmd(ADC1,ENABLE);//��ʼADCת��
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);//�жϱ�־λת���Ƿ����
	
	data = ADC_GetConversionValue(ADC1);//��ȡ���ݺ���Զ����ת����ɱ�־λ
    return ((2400 - (data-800))/ 2400.00)*100;
}
