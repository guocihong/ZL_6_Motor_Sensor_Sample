#include "driver/uart/uart_drv.h"

#define S2TI   0x02
#define S2RI   0x01

/* UART2 */
extern xdata  Byte     recv2_buf[MAX_RecvFrame];    // receiving buffer
extern idata  Byte     recv2_state;                 // receive state
extern idata  Byte     recv2_timer;                 // receive time-out, �����ֽڼ䳬ʱ�ж�
extern idata  Byte     recv2_chksum;                // computed checksum
extern idata  Byte     recv2_ctr;                   // reveiving pointer

extern xdata  Byte     trans2_buf[MAX_TransFrame];  // uart transfer message buffer
extern idata  Byte     trans2_ctr;                  // transfer pointer
extern idata  Byte     trans2_size;                 // transfer bytes number
extern idata  Byte     trans2_chksum;               // computed check-sum of already transfered message

extern  data  Byte     uart2_q_index;               // ���ڷ���ĳ���������ţ���Ϊ0xFF, ��ʾû���κ�����뷢������
extern xdata  sUART_Q  uart2_send_queue[UART_QUEUE_NUM];     // ���ڷ��Ͷ���
extern xdata  sUART_Q  uart2_recv_queue[UART_QUEUE_NUM];     // ���ڽ��ն���

extern Byte uart2_get_recv_buffer(void);

void uart_init(void)    //9600bps@22.1184MHz
{
	//uart2Ӳ����ʼ��
	AUXR1 |= 0x10;      //����2��P1���л���P4��
	S2CON = 0x50;		//8λ����,�ɱ䲨����,����ʹ��
	AUXR &= 0xF7;		//�����ʲ�����
	AUXR &= 0xFB;		//���������ʷ�����ʱ��ΪFosc/12,��12T
	BRT = 0xFA;		    //�趨���������ʷ�������װֵ
	AUXR |= 0x10;		//�������������ʷ�����
	IE2 |= 0x01;        //ʹ�ܴ���2�ж�
	IP2H &= 0xFE;       //�����ж����ȼ�Ϊ���ȼ�1
	IP2 |= 0x01;
}

void uart2_isr(void) interrupt SIO2_VECTOR using 1
{
	Byte c,i;

	if (S2CON & S2TI) { //UART2�����ж�
		trans2_ctr++;   //ȡ��һ��������index
		if (trans2_ctr < trans2_size) { //δ�������
			if (trans2_ctr == (trans2_size - 1)) { //�Ѿ�ָ��У���ֽ�
				S2BUF = trans2_chksum;    //����У���ֽ�
			} else { //��У���ֽ�, ��Ҫ���Ͳ�����checksum
				S2BUF = trans2_buf[trans2_ctr];
				if (trans2_ctr > 0) { //����check_sum
					trans2_chksum += trans2_buf[trans2_ctr];   //����chksum
				}
			}
		} else { //�Ѿ�ȫ���������(��У���ֽ�)�������÷���������
			//Ŀǰ��ƣ�������ȴ�Ӧ��, �����ͷŸö�����
			if (uart2_q_index < UART_QUEUE_NUM) {
				uart2_send_queue[uart2_q_index].flag = 0;   //�ö��������
			}
			
            uart2_q_index = 0xFF;	   //�޶������ڷ���
		}
        
		S2CON &= ~S2TI;   //must clear by user software
	}

	if (S2CON & S2RI) { //�����ж�
		c = S2BUF;
		switch (recv2_state)
		{
		case FSA_INIT://�Ƿ�Ϊ֡ͷ
			if (c == FRAME_STX) { //Ϊ֡ͷ, ��ʼ�µ�һ֡
				recv2_ctr = 0;
				recv2_chksum = 0;
				recv2_timer = RECV_TIMEOUT;
				recv2_state = FSA_ADDR_D;
			}
			break;

		case FSA_ADDR_D://ΪĿ�ĵ�ַ, ��ʼ���沢����Ч���
			recv2_buf[recv2_ctr++] = c;
			recv2_chksum += c;
			recv2_timer = RECV_TIMEOUT;
			recv2_state = FSA_ADDR_S;
			break;

		case FSA_ADDR_S://ΪԴ��ַ
			recv2_buf[recv2_ctr++] = c;
			recv2_chksum += c;
			recv2_timer = RECV_TIMEOUT;
			recv2_state = FSA_LENGTH;
			break;

		case FSA_LENGTH://Ϊ�����ֽ�
			if ((c > 0) && (c < (MAX_RecvFrame - 3))) { //��Ч��
				recv2_buf[recv2_ctr++] = c;    //�������ֽڱ��泤��
				recv2_chksum += c;
				recv2_timer = RECV_TIMEOUT;
				recv2_state = FSA_DATA;
			} else {	//����Ч��
				recv2_state = FSA_INIT;
			}
			break;

		case FSA_DATA://��ȡ���
			recv2_buf[recv2_ctr] = c;
			recv2_chksum += c;   //����У���
			if (recv2_ctr == (recv2_buf[2] + 2)) { //�Ѿ��յ�ָ�����ȵ���������
				recv2_state = FSA_CHKSUM;
			} else {	//��δ����
				recv2_ctr++;
			}
			recv2_timer = RECV_TIMEOUT;
			break;

		case FSA_CHKSUM://���У���ֽ�
			if (recv2_chksum == c) {//�Ѿ��յ�����һ֡
                i = uart2_get_recv_buffer();
				if (i < UART_QUEUE_NUM) {//�ҵ��˿���buffer, д��data
                    memcpy(uart2_recv_queue[i].tdata, recv2_buf, recv2_buf[2] + 3);
                }
			}
		default://��λ
			recv2_state = FSA_INIT;
			break;
		}
        
		S2CON &= ~S2RI;     //must clear by user software
	}
}

void uart2_start_trans(void)
{ 
	trans2_chksum = 0;
	trans2_ctr = 0;
	S2BUF = trans2_buf[trans2_ctr];
}