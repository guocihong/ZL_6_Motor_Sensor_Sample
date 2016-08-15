#include "task/doorkeep/doorkeep_task.h"

#define DK_CHECK_INTERVAL       (1000 / SCHEDULER_TICK)	          //�Ŵſ��ؼ������
#define DK_CONFORM_DELAY        ( 100 / SCHEDULER_TICK)            //�Ŵſ��ر仯ȷ����ʱ

/* Doorkeep(�Ŵ�) */
extern bdata  bit          gl_dk_status;                //�Ŵſ���״̬��ÿ1s��̬��⣩: 1 - �պ�; 0 - ��(��Ҫ����)                    
extern xdata  Byte         gl_dk_tick;  	            //�Ŵż���ʱtick

static xdata  Byte         dk_read_state;               //task state

void doorkeep_task_init(void)
{
	gl_dk_tick    = 0;
	gl_dk_status  = 1;                                   //�ϵ�ʱ��ȱʡΪ�պ�
	dk_read_state = DK_READ_START;
}

void doorkeep_task(void)
{
	switch (dk_read_state)
	{
	case DK_READ_START: //��ʼ����
		dk_read_state = DK_READ_IDLE;
		break;

	case DK_READ_IDLE://����Ŵ����ޱ仯
		if (gl_dk_tick > DK_CHECK_INTERVAL) {
			//ÿ����һ��
			gl_dk_tick = 0;
			if (gl_dk_status == 1) {
				//ԭ��Ϊ�Ŵűպ�
				if (bDoorKeeper == 0) {
					//�Ŵſ��ܱ���, ������ʱȷ�Ͻ׶�
					dk_read_state = DK_READ_DELAY;
				}
			} else {
				//ԭ��Ϊ�ŴŴ�
				if (bDoorKeeper == 1) {
					//�Ŵſ��ܱ��պ�, ������ʱȷ�Ͻ׶�
					dk_read_state = DK_READ_DELAY;
				}
			}
		}
		break;

	case DK_READ_DELAY://��ʱȷ��
		if (gl_dk_tick > DK_CONFORM_DELAY) {
			// ��ʱʱ�䵽, �ж��Ƿ����ȶ��ı仯
			if ((gl_dk_status == 1) && (bDoorKeeper == 0)) {// �Ŵ��Ѿ���				
				//���±���
				gl_dk_status = 0;
			} else if ((gl_dk_status == 0) && (bDoorKeeper == 1)) {// �Ŵ��Ѿ��պ�
				//���±���
				gl_dk_status = 1;
			}

			gl_dk_tick = 0;
			dk_read_state = DK_READ_IDLE;
		}
		break;
	}
}
