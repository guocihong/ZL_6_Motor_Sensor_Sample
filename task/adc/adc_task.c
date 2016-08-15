#include "task/adc/adc_task.h"

/* AD sample */
#define AD_EQU_PNUM             4                                 //ÿ����˿����4����ƽ��ֵ

extern idata  Byte         ad_index;                    //���ڲ�����ͨ����, ȡֵ��Χ0~12
extern  data  sAD_Sample   ad_sample;                   //���浱ǰ����ֵ
extern idata  sAD_Sum      ad_samp_equ[13];             //����ȥ�������
extern xdata  Union16      ad_chn_sample[13];           //����һ�ֲ���ֵ���Ѿ���ȥ������ÿͨ��һ���㣬ѭ�����棩

/* UART2 */
extern  data  Byte         uart2_q_index;               // ���ڷ���ĳ���������ţ���Ϊ0xFF, ��ʾû���κ�����뷢������
extern xdata  sUART_Q      uart2_send_queue[UART_QUEUE_NUM];     // ���ڷ��Ͷ���

/* Doorkeep(�Ŵ�) */
extern bdata  bit          gl_dk_status;                //�Ŵſ���״̬��ÿ1s��̬��⣩: 1 - �պ�; 0 - ��(��Ҫ����)                    

/* ������� */
extern bdata  bit          gl_motor_overcur_flag;       //����Ƿ��ڶ�ת״̬��0-��������;1-�����ת
extern bdata  bit          gl_motor_adjust_flag;        //����Ƿ��ڹ���״̬��0-ֹͣ����״̬;1-�����ڹ���״̬
extern bdata  bit          is_timeout;                  //���ת��ʱ�����꣺0-û��;1-ʱ������

extern Byte uart2_get_send_buffer(void);

void adc_task_init(void)
{
	Byte i;

	//��ر�����ʼ��
	ad_index        = 0;
	ad_sample.valid = 0;                     //���У�����д����ֵ
	for (i = 0; i < 13; i++)
	{
		ad_samp_equ[i].sum       = 0;        //����ȥ�������
		ad_samp_equ[i].point     = 0;
		ad_chn_sample[i].w       = 0;        //����һ�ֲ���ֵ
	}

    is_timeout = 0;
    
	//adcӲ����ʼ��
	adc_init();
}

void adc_task(void)
{
	Byte    i,j;
	Byte    index;          //����ͨ����
	Uint16  val_temp;       //�������10bit����ֵ,  ������ʱ����
	Uint16  val;            //4������õ���ƽ������ֵ, ��Ϊһ���ɽ��г����жϵ���С��

	if (ad_sample.valid)    //���²������ݵ���
	{
		// 0. ���浽��ʱ����
		val_temp = ad_sample.val;
		index    = ad_sample.index;

		// 1. ���浽����ȥ���������
		ad_samp_equ[index].sum += val_temp;
		ad_samp_equ[index].point++;

		// 2. ��ǰͨ���Ƿ�����ȥ��������
		if (ad_samp_equ[index].point == AD_EQU_PNUM)
		{
			// ����ȥ���������������������һ����
			// 2.a �����Ӧͨ����һ��������
			val = ad_samp_equ[index].sum >> 2;  //����4

			// 2.b ���㵱ǰͨ����ȥ������ͽṹ
			ad_samp_equ[index].sum = 0;
			ad_samp_equ[index].point = 0;

			// 2.c ����ʵʱ����ֵ
			ad_chn_sample[index].w = val;   //���浽����һ�ֲ���ֵ������

			// 2.d ʵʱ���Ͳ���ֵ
			if (index == 12)
			{ //�Ѿ����13��ͨ����һ�������������ɷ��Ͳ���ֵ
				i = uart2_get_send_buffer();
				if (i < UART_QUEUE_NUM)
				{ //�ҵ��˿���buffer, д��data
					uart2_send_queue[i].tdata[0] = FRAME_STX;
					uart2_send_queue[i].tdata[1] = 0xFF;
					uart2_send_queue[i].tdata[2] = 0x00;
					
					uart2_send_queue[i].tdata[3] = 0x21;
					uart2_send_queue[i].tdata[4] = 0xE8;
					uart2_send_queue[i].tdata[5] = 0x1F;
					uart2_send_queue[i].tdata[6] = 0x1C;

					//��ch1 ~ ch6
					for (j = 0; j < 6; j++)
					{
						uart2_send_queue[i].tdata[7 + (j << 1)] = HIGH(ad_chn_sample[j].w);
						uart2_send_queue[i].tdata[8 + (j << 1)] = LOW(ad_chn_sample[j].w);
					}
					
					//��ch1 ~ ch6
					for (j = 0; j < 6; j++)
					{
						uart2_send_queue[i].tdata[19 + (j << 1)] = HIGH(ad_chn_sample[6 + j].w);
						uart2_send_queue[i].tdata[20 + (j << 1)] = LOW(ad_chn_sample[6 + j].w);
					}

					//������
					uart2_send_queue[i].tdata[31] = HIGH(ad_chn_sample[12].w);
					uart2_send_queue[i].tdata[32] = LOW(ad_chn_sample[12].w);

					//�Ŵ�״̬
					uart2_send_queue[i].tdata[33] = (Byte)gl_dk_status;
					
					//�����ת״̬
					uart2_send_queue[i].tdata[34] = (Byte)gl_motor_adjust_flag;
					
					//�����ת״̬
					uart2_send_queue[i].tdata[35] = (Byte)gl_motor_overcur_flag;
					
                    //ʱ���Ƿ�����
                    uart2_send_queue[i].tdata[36] = (Byte)is_timeout;

					uart2_send_queue[i].len       = 38;
                    
                    gl_motor_overcur_flag         = 0;
                    is_timeout                    = 0;
				}
				else
				{   //�޿���buffer, ����������
					//���: ���ж��������ڷ���, �ȴ������
					while (uart2_q_index != 0xFF);	//������,������ WDT ��λ
				}
			}
		}

		//3.��ǰ����ֵ������ϣ������µĲ���ֵ����
		ad_sample.valid = FALSE;
	}
}