#include "sys.h"

#include "stm32f10x.h"
#include "stm32f10x_adc.h"


#include "adc1.h"

void adc1_init(void)
{ 
    GPIO_InitTypeDef     GPIO_InitStructure;//初始化采集ADC的脚
    ADC_InitTypeDef     ADC_InitStructure;//配置ADC结构体
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//开启ADC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
    
    //不要忘记配置ADCCLK时钟预分频
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//配置ADC时钟预分频 ADCCLK=72Mhz/6=12Mhz
    
    //配置模拟输入引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//配置成模拟输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //选择引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;// 配置引脚速率
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    //配置规则组
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_55Cycles5);
    
    //配置ADC结构体初始化
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//配置ADC模式是双通道模式还是单通道模式
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据对齐模式 左对齐
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None ;  //外部触发转换选择就是触发控制的触发源  ， 使用内部触发（软件触发 ）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //连续转换模式，可以选择是连续转换还是单次转换  DISABLE单次模式  ENABLE连续模式
    ADC_InitStructure.ADC_NbrOfChannel = 1; //通道数目，在指定扫描模式下，总共会用到几个通道
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; //扫描转换模式，可以选择是扫描模式还是非扫描模式        DISABLE非扫描模式，   ENABLE扫描模式
    ADC_Init(ADC1,&ADC_InitStructure);
    
    //开启ADC电源
    ADC_Cmd(ADC1,ENABLE);
    
    //开始ADC校准
    ADC_ResetCalibration(ADC1); //开始复位校准 ，标志位置1硬件清零
    while(ADC_GetCalibrationStatus(ADC1)==SET); //判断校准标志位置是否为 ，为0校准完成
    ADC_StartCalibration(ADC1);    //开始校准
    while(ADC_GetCalibrationStatus(ADC1)==SET);//判断是否结束校准
}

// 获取土壤湿度百分比估值 不保证十分准确
uint8_t adc_get_soil_moist(void)
{
	uint16_t data;
    ADC_SoftwareStartConvCmd(ADC1,ENABLE);//开始ADC转换
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);//判断标志位转换是否完成
	
	data = ADC_GetConversionValue(ADC1);//读取数据后会自动清除转换完成标志位
    return ((2400 - (data-800))/ 2400.00)*100;
}
