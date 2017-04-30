#include "task/motor/motor_task.h"

#define MT_CONFORM_DELAY1       (30 / SCHEDULER_TICK)             //���ȷ����ʱ1
#define MT_CONFORM_DELAY2       (400 / SCHEDULER_TICK)            //���ȷ����ʱ2

/* ����״̬ */
#define MT_READ_START           0
#define MT_READ_IDLE            1
#define MT_READ_DELAY1          2
#define MT_READ_DELAY2          3

/* ������� */
extern xdata  Byte         gl_motor_overcur_tick;       //�����ת����ʱtick
extern bdata  bit          gl_motor_overcur_flag;       //����Ƿ��ڶ�ת״̬��0-��������;1-�����ת
extern bdata  bit          gl_motor_adjust_flag;        //����Ƿ��ڹ���״̬��0-ֹͣ����״̬;1-�����ڹ���״̬
static xdata  Byte         mt_read_state;               //task state

void motor_task_init(void)
{
	gl_motor_overcur_tick = 0;
	gl_motor_overcur_flag = 0;                          //�ϵ�ʱ��ȱʡΪ��������,�������ת
	mt_read_state         = MT_READ_START;
}

void motor_task(void)
{
	switch (mt_read_state)
	{
	case MT_READ_START: //��ʼ����
		mt_read_state = MT_READ_IDLE;
		break;

	case MT_READ_IDLE:
		if (gl_motor_adjust_flag == 1) {//ֻ�е������ʱ���ż�����Ƿ��ת
			if (bMotorOverCur == 0) {//������ܱ���ת��������ʱȷ�Ͻ׶�
				gl_motor_overcur_tick = MT_CONFORM_DELAY1;
				mt_read_state = MT_READ_DELAY1;
			}
		}
		break;

	case MT_READ_DELAY1://��ʱȷ��
		if (gl_motor_overcur_tick == 0) {
			// ��ʱʱ�䵽��ǰ30ms�͵�ƽ���账����Ϊ����ë�̸���
			mt_read_state = MT_READ_DELAY2;
			gl_motor_overcur_tick = MT_CONFORM_DELAY2;
		}
		break;
		
	case MT_READ_DELAY2://��ʱȷ��
		if (gl_motor_overcur_tick == 0) {
			// ��ʱʱ�䵽, �ж��Ƿ����ȶ��ı仯
			if (bMotorOverCur == 0) {//���ȷʵΪ��ת��ֹͣ���               
                //ֹͣ���
                P15 = 0;
                P16 = 0;
                P2  = 0;
                P0  = 0;
				
				//��λ
				gl_motor_adjust_flag  = 0;

				//��λ
				gl_motor_overcur_flag = 1;
			}
			
			//��λ
			mt_read_state = MT_READ_IDLE;
		}
		break;
	}
}