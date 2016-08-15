#include "driver/uart/uart_drv.h"

#define S2TI   0x02
#define S2RI   0x01

/* UART2 */
extern xdata  Byte     recv2_buf[MAX_RecvFrame];    // receiving buffer
extern idata  Byte     recv2_state;                 // receive state
extern idata  Byte     recv2_timer;                 // receive time-out, 用于字节间超时判定
extern idata  Byte     recv2_chksum;                // computed checksum
extern idata  Byte     recv2_ctr;                   // reveiving pointer

extern xdata  Byte     trans2_buf[MAX_TransFrame];  // uart transfer message buffer
extern idata  Byte     trans2_ctr;                  // transfer pointer
extern idata  Byte     trans2_size;                 // transfer bytes number
extern idata  Byte     trans2_chksum;               // computed check-sum of already transfered message

extern  data  Byte     uart2_q_index;               // 正在发送某队列项的序号：若为0xFF, 表示没有任何项进入发送流程
extern xdata  sUART_Q  uart2_send_queue[UART_QUEUE_NUM];     // 串口发送队列
extern xdata  sUART_Q  uart2_recv_queue[UART_QUEUE_NUM];     // 串口接收队列

extern Byte uart2_get_recv_buffer(void);

void uart_init(void)    //9600bps@22.1184MHz
{
	//uart2硬件初始化
	AUXR1 |= 0x10;      //串口2从P1口切换到P4口
	S2CON = 0x50;		//8位数据,可变波特率,接收使能
	AUXR &= 0xF7;		//波特率不倍速
	AUXR &= 0xFB;		//独立波特率发生器时钟为Fosc/12,即12T
	BRT = 0xFA;		    //设定独立波特率发生器重装值
	AUXR |= 0x10;		//启动独立波特率发生器
	IE2 |= 0x01;        //使能串口2中断
	IP2H &= 0xFE;       //设置中断优先级为优先级1
	IP2 |= 0x01;
}

void uart2_isr(void) interrupt SIO2_VECTOR using 1
{
	Byte c,i;

	if (S2CON & S2TI) { //UART2发送中断
		trans2_ctr++;   //取下一个待传送index
		if (trans2_ctr < trans2_size) { //未传送完成
			if (trans2_ctr == (trans2_size - 1)) { //已经指向校验字节
				S2BUF = trans2_chksum;    //发送校验字节
			} else { //非校验字节, 需要传送并计算checksum
				S2BUF = trans2_buf[trans2_ctr];
				if (trans2_ctr > 0) { //计算check_sum
					trans2_chksum += trans2_buf[trans2_ctr];   //更新chksum
				}
			}
		} else { //已经全部传送完成(含校验字节)，可以置发送器空闲
			//目前设计：均不需等待应答, 可以释放该队列项
			if (uart2_q_index < UART_QUEUE_NUM) {
				uart2_send_queue[uart2_q_index].flag = 0;   //该队列项空闲
			}
			
            uart2_q_index = 0xFF;	   //无队列项在发送
		}
        
		S2CON &= ~S2TI;   //must clear by user software
	}

	if (S2CON & S2RI) { //接收中断
		c = S2BUF;
		switch (recv2_state)
		{
		case FSA_INIT://是否为帧头
			if (c == FRAME_STX) { //为帧头, 开始新的一帧
				recv2_ctr = 0;
				recv2_chksum = 0;
				recv2_timer = RECV_TIMEOUT;
				recv2_state = FSA_ADDR_D;
			}
			break;

		case FSA_ADDR_D://为目的地址, 开始保存并计算效验和
			recv2_buf[recv2_ctr++] = c;
			recv2_chksum += c;
			recv2_timer = RECV_TIMEOUT;
			recv2_state = FSA_ADDR_S;
			break;

		case FSA_ADDR_S://为源地址
			recv2_buf[recv2_ctr++] = c;
			recv2_chksum += c;
			recv2_timer = RECV_TIMEOUT;
			recv2_state = FSA_LENGTH;
			break;

		case FSA_LENGTH://为长度字节
			if ((c > 0) && (c < (MAX_RecvFrame - 3))) { //有效串
				recv2_buf[recv2_ctr++] = c;    //第三个字节保存长度
				recv2_chksum += c;
				recv2_timer = RECV_TIMEOUT;
				recv2_state = FSA_DATA;
			} else {	//非有效串
				recv2_state = FSA_INIT;
			}
			break;

		case FSA_DATA://读取命令串
			recv2_buf[recv2_ctr] = c;
			recv2_chksum += c;   //更新校验和
			if (recv2_ctr == (recv2_buf[2] + 2)) { //已经收到指定长度的命令数据
				recv2_state = FSA_CHKSUM;
			} else {	//还未结束
				recv2_ctr++;
			}
			recv2_timer = RECV_TIMEOUT;
			break;

		case FSA_CHKSUM://检查校验字节
			if (recv2_chksum == c) {//已经收到完整一帧
                i = uart2_get_recv_buffer();
				if (i < UART_QUEUE_NUM) {//找到了空闲buffer, 写入data
                    memcpy(uart2_recv_queue[i].tdata, recv2_buf, recv2_buf[2] + 3);
                }
			}
		default://复位
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