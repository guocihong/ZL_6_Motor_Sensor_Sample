#include "driver/adc/adc_drv.h"

void adc_init(void)
{
    //����P1.7��ΪAD����
    P1ASF |= 0x80;
    
	//��AD��Դ
    //����ADת�����ʣ�540��ʱ������ת��һ��
	//ѡ��ͨ��P17
    ADC_CONTR = ADC_POWER_ENABLE | ADC_SPEED_540 | ADC_SAMPLE_CHANNEL;
    
	//�ر�ADC�ж�
	EADC = 0;
	
    //����ADת������洢��ʽ
    AUXR1 &= 0xFB;//10λADת�������8λ�洢��ADC_RES,��2λ�洢��ADC_RESL��
}
