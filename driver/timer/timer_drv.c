#include "driver/timer/timer_drv.h"
#include "driver/adc/adc_drv.h"

/* UART2 */
extern idata  Byte         recv2_state;                 // receive state
extern idata  Byte         recv2_timer;                 // receive time-out, �����ֽڼ䳬ʱ�ж�

/* AD sample */
extern  data  Byte         ad_index;                    //���ڲ�����ͨ����, ȡֵ��Χ0~12
extern  data  sAD_Sample   ad_sample;                   //���浱ǰ����ֵ

/* Doorkeep(�Ŵ�) */
extern xdata  Byte         gl_dk_tick;  	            //�Ŵż���ʱtick

/* ������� */ 
extern xdata  Byte         gl_motor_overcur_tick;       //�����ת����ʱtick                                          
extern xdata  Uint16       motor_run_tick;              //���ת����ʱtick,��λΪ10ms,��Ϊ0,���ֹͣ
extern bdata  bit          gl_motor_adjust_flag;        //����Ƿ��ڹ���״̬��0-ֹͣ����״̬;1-�����ڹ���״̬
extern bdata  bit          is_timeout;                  //���ת��ʱ�����꣺0-û��;1-ʱ������

/* ����������ƫ�� */
extern xdata  Uint16       sensor_sample_offset[13];    //����������ƫ�û������ʱ������������ֵ��Ϊ0����Լ400���ң���Ҫ������˲������ = ����ֵ - ����ƫ��

void timer0_init(void)   // 5ms@22.1184MHz
{    
    // ��ʱ��0��ʼ��
	AUXR &= 0x7F;		 // ����Ϊ12Tģʽ
	TMOD &= 0xF0;		 // ����Ϊ����ģʽ1
	TMOD |= 0x01;
	TL0 = 0x00;		     // ���ö�ʱ��ֵ
	TH0 = 0xDC;		     // ���ö�ʱ��ֵ
	TF0 = 0;		     // ���TF0��־
    ET0 = 1;             // ʹ��T0�ж�����λ
    IPH |= (1 << 1);
    PT0 = 0;             // �����ж����ȼ�Ϊ���ȼ�2
	TR0 = 1;		     // ��ʱ��0��ʼ��ʱ
	
	// ����ADת��
	P5 = ad_index;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
	ADC_CONTR |= ADC_START;
}

void timer0_isr(void) interrupt TF0_VECTOR using 0
{	               
    // ��װ��ֵ
    TR0 = 0;
	TL0 = 0x00;		                              // ���ö�ʱ��ֵ
	TH0 = 0xDC;		                              // ���ö�ʱ��ֵ
	TR0 = 1;		                              // ��ʱ��0��ʼ��ʱ
        
    // ADת�����,��ADC_FLAGת����ɱ�־����
    ADC_CONTR &= ~ADC_FLAG;

	// ��ADֵ
	if (ad_sample.valid == FALSE) {
		// ԭ�����Ѿ�������, ����д��������
		ad_sample.val   = ADC_RES;                // ����8λ
		ad_sample.val   = ad_sample.val << 2;
		ad_sample.val   += (ADC_RESL & 0x03);     // �õ�10bit����ֵ
		ad_sample.index = ad_index;
		ad_sample.valid = TRUE;
			
		//��6����˿����6����˿�Ĳ���ֵ��Ҫ��ȥ����ƫ���Լ�������Ĳ���ֵ��Ҫ��ȥ����ƫ��
        if ((ad_index >= 0) && (ad_index <= 11)) {
            if (ad_sample.val > sensor_sample_offset[ad_index]) {
                ad_sample.val -= sensor_sample_offset[ad_index];
            } else {
                ad_sample.val = 0;
            }
        }
        
		// ������һͨ������
		if (ad_index >= 12) {
			ad_index = 0;
		} else {
			ad_index++;
		}

		P5 = ad_index;	                          // ѡ��ģ������
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
   		ADC_CONTR = ADC_POWER_ENABLE | ADC_SPEED_540 | ADC_SAMPLE_CHANNEL; // ����ת��
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        ADC_CONTR |=  ADC_START;
	}

	// increment task tick counters
	gl_dk_tick++;                                 //�Ŵż���ʱtick
    if (gl_motor_overcur_tick > 0) {
    	gl_motor_overcur_tick--;                      //�����ת��ʱtick
    }
	 
	if (motor_run_tick > 0) {
		motor_run_tick--;
		
		if (motor_run_tick == 0) {                //ʱ�䵽,ֹͣ���
			P15 = 0;
			P16 = 0;
			P2  = 0;
			P0  = 0;
            gl_motor_adjust_flag = 0;
            is_timeout = 1;
		}
	}
	
	// UART2�ֽ�֮����ճ�ʱ
	if (recv2_state != FSA_INIT) { 
		//�ǳ�ʼ״̬����Ҫ����Ƿ�ʱ
		if (recv2_timer > 0) {
			recv2_timer--;
		}
		
		if (recv2_timer == 0) {
			recv2_state = FSA_INIT;               //���ճ�ʱ, �ָ�����ʼ״̬			
		}
	}
}