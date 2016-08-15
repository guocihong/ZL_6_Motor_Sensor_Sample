#include "driver/adc/adc_drv.h"

void adc_init(void)
{
    //设置P1.7做为AD功能
    P1ASF |= 0x80;
    
	//打开AD电源
    //设置AD转换速率：540个时钟周期转换一次
	//选中通道P17
    ADC_CONTR = ADC_POWER_ENABLE | ADC_SPEED_540 | ADC_SAMPLE_CHANNEL;
    
	//关闭ADC中断
	EADC = 0;
	
    //设置AD转换结果存储格式
    AUXR1 &= 0xFB;//10位AD转换结果高8位存储在ADC_RES,低2位存储在ADC_RESL中
}
