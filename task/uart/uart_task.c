#include "task/uart/uart_task.h"

/* UART2 */
extern xdata  Byte         recv2_buf[MAX_RecvFrame];    // receiving buffer
extern idata  Byte         recv2_state;                 // receive state
extern idata  Byte         recv2_timer;                 // receive time-out, �����ֽڼ䳬ʱ�ж�
extern idata  Byte         recv2_chksum;                // computed checksum
extern idata  Byte         recv2_ctr;                   // reveiving pointer

extern xdata  Byte         trans2_buf[MAX_TransFrame];  // uart transfer message buffer
extern idata  Byte         trans2_ctr;                  // transfer pointer
extern idata  Byte         trans2_size;                 // transfer bytes number
extern idata  Byte         trans2_chksum;               // computed check-sum of already transfered message

extern  data  Byte         uart2_q_index;               // ���ڷ���ĳ���������ţ���Ϊ0xFF, ��ʾû���κ�����뷢������
extern xdata  sUART_Q      uart2_send_queue[UART_QUEUE_NUM];     // ���ڷ��Ͷ���
extern xdata  sUART_Q      uart2_recv_queue[UART_QUEUE_NUM];     // ���ڽ��ն���

/* AD sample */
extern  data  Union16      ad_chn_sample[13];           //����һ�ֲ���ֵ���Ѿ���ȥ������ÿͨ��һ���㣬ѭ�����棩

/* ������� */                                          
extern xdata  const  Byte  Motor_Control_Code[2][12];
extern xdata  Byte         motor_index;                 //�������0-11
extern xdata  Byte         motor_run_mode;              //��ת(1),��ת(2),ֹͣ(0)
extern xdata  Uint16       motor_run_tick;              //���ת����ʱtick,��λΪ1s
extern bdata  bit          gl_motor_adjust_flag;        //����Ƿ��ڹ���״̬��0-ֹͣ����״̬;1-�����ڹ���״̬
extern bdata  bit          gl_motor_overcur_flag;       //����Ƿ��ڶ�ת״̬��0-��������;1-�����ת
extern bdata  bit          is_timeout;                  //���ת��ʱ�����꣺0-û��;1-ʱ������

/* ����������ƫ�� */
extern xdata  Uint16       sensor_sample_offset[13];    //����������ƫ�û������ʱ������������ֵ��Ϊ0����Լ400���ң���Ҫ������˲������ = ����ֵ - ����ƫ��

void uart_task_init(void)
{
	Byte i;

	//uart2��ر�����ʼ��
	recv2_state    = FSA_INIT;
	recv2_timer    = 0;
	recv2_ctr      = 0;
	recv2_chksum   = 0;

	trans2_size    = 0;
	trans2_ctr     = 0;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		uart2_send_queue[i].flag = 0; //������
		uart2_recv_queue[i].flag = 0; //������
	}
	uart2_q_index = 0xFF;             //�޶�������뷢������

	//UARTӲ����ʼ��
	uart_init();                      //�Ѿ�׼���ô����շ���ֻ�ǻ�δʹ��ȫ���ж�
}

void uart_task(void)
{
	Byte i,j;
	Byte *ptr;
	
	//1. UART2 ���д������Է�ˮ��������
	//���Ƿ��еȴ��������
	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		if (uart2_recv_queue[i].flag == 1)//�еȴ��������
		{
			ptr = uart2_recv_queue[i].tdata;
			
			//����
			switch (ptr[3])
			{
			case CMD_ZL_PRE://����/����ר�������־
				
				switch (ptr[5])
				{
				case 0xF0://���Ƶ����ת/��ת/ֹͣ
					motor_index    = ptr[6];
					motor_run_mode = ptr[7];
					motor_run_tick = ptr[8] * 2 * 100;
                
					//���Ƶ���Ĵ���
					P2 = Motor_Control_Code[0][motor_index];
					P0 = Motor_Control_Code[1][motor_index];
					
					if (motor_run_mode == 1) {//�����ת
                        gl_motor_adjust_flag  = 1;
                        gl_motor_overcur_flag = 0;
						P15 = 1;
						P16 = 0;
                        is_timeout = 0;
					} else if (motor_run_mode == 2) {//�����ת
                        gl_motor_adjust_flag  = 1;
                        gl_motor_overcur_flag = 0; 
						P15 = 0;
						P16 = 1;
                        is_timeout = 0;
					} else if (motor_run_mode == 0) {//���ֹͣ
                        gl_motor_adjust_flag  = 0;
                        gl_motor_overcur_flag = 0;
						P15 = 0;
						P16 = 0;
						P2  = 0;
						P0  = 0;
                        is_timeout = 1;
					}
					break;
					
				case 0xF1: //���ô���������ƫ��---->������·�ϵ����                  
					//1. д��flash�����±���
					flash_enable();
					flash_erase(EEPROM_SECTOR3);

					for (j = 0; j < 6; j++) {
						//��1 ~6
						sensor_sample_offset[j] =
						    ((Uint16)ptr[6 + (j << 1)] << 8) + ptr[7 + (j << 1)];
						flash_write(ptr[6 + (j << 1)], EEPROM_SECTOR3 + 1 + (j << 1));
						flash_write(ptr[7 + (j << 1)], EEPROM_SECTOR3 + 2 + (j << 1));
					}

					for (j = 0; j < 6; j++) {
						//��1 ~6
						sensor_sample_offset[6 + j] =
						    ((Uint16)ptr[22 + (j << 1)] << 8) + ptr[23 + (j << 1)];
						flash_write(ptr[22 + (j << 1)], EEPROM_SECTOR3 + 13 + (j << 1));
						flash_write(ptr[23 + (j << 1)], EEPROM_SECTOR3 + 14 + (j << 1));
					}

					//������
                    sensor_sample_offset[12] = 0;
						
					flash_write(0x5a, EEPROM_SECTOR3);
					flash_disable(); 

					break;
				}
				
				break;
			}

			//�������,�ͷŸö�����
			uart2_recv_queue[i].flag = 0;
			break;
		}
	}

	//2. UART2 ���з��ͣ�������������͸���ˮ��
	if (uart2_q_index == 0xFF)
	{
		//UART2�޽��뷢�����̵Ķ�����, ���Ƿ��еȴ����͵���
		for (i = 0; i < UART_QUEUE_NUM; i++)
		{
			if (uart2_send_queue[i].flag == 1)
			{
				//�еȴ����͵�����Ŵ����
				uart2_send_queue[i].flag = 2;
				uart2_q_index = i;
				memcpy(trans2_buf, uart2_send_queue[i].tdata, uart2_send_queue[i].len - 1);
				trans2_size = uart2_send_queue[i].len;
				uart2_start_trans();
				break;
			}
		}
	}
}

/***************************************************************************
* NAME: uart2_get_send_buffer
*----------------------------------------------------------------------------
* PARAMS:
* return: Byte
*         ������ֵ >= UART_QUEUE_NUM, ���ʾû�����뵽����buffer
*----------------------------------------------------------------------------
* PURPOSE: �ڴ���2������Ѱ�ҿ��ж�������ҵ������ض��������(0 ~ (UART_QUEUE_NUM-1))
*****************************************************************************/
Byte uart2_get_send_buffer(void)
{
	Byte i, flag;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		Disable_interrupt();
		flag = uart2_send_queue[i].flag;
		Enable_interrupt();
		if (flag == 0)   //���ҵ�����Buffer
		{
			uart2_send_queue[i].flag = 1;
			break;
		}
	}
	return i;
}

/***************************************************************************
* NAME: uart2_get_recv_buffer
*----------------------------------------------------------------------------
* PARAMS:
* return: Byte
*         ������ֵ >= UART_QUEUE_NUM, ���ʾû�����뵽����buffer
*----------------------------------------------------------------------------
* PURPOSE: �ڴ���2������Ѱ�ҿ��ж�������ҵ������ض��������(0 ~ (UART_QUEUE_NUM-1))
*****************************************************************************/
Byte uart2_get_recv_buffer(void)
{
	Byte i, flag;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		Disable_interrupt();
		flag = uart2_recv_queue[i].flag;
		Enable_interrupt();
		if (flag == 0)   //���ҵ�����Buffer
		{
			uart2_recv_queue[i].flag = 1;
			break;
		}
	}
	return i;
}