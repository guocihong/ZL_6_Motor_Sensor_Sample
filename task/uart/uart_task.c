#include "task/uart/uart_task.h"

/* UART2 */
extern xdata  Byte         recv2_buf[MAX_RecvFrame];    // receiving buffer
extern idata  Byte         recv2_state;                 // receive state
extern idata  Byte         recv2_timer;                 // receive time-out, 用于字节间超时判定
extern idata  Byte         recv2_chksum;                // computed checksum
extern idata  Byte         recv2_ctr;                   // reveiving pointer

extern xdata  Byte         trans2_buf[MAX_TransFrame];  // uart transfer message buffer
extern idata  Byte         trans2_ctr;                  // transfer pointer
extern idata  Byte         trans2_size;                 // transfer bytes number
extern idata  Byte         trans2_chksum;               // computed check-sum of already transfered message

extern  data  Byte         uart2_q_index;               // 正在发送某队列项的序号：若为0xFF, 表示没有任何项进入发送流程
extern xdata  sUART_Q      uart2_send_queue[UART_QUEUE_NUM];     // 串口发送队列
extern xdata  sUART_Q      uart2_recv_queue[UART_QUEUE_NUM];     // 串口接收队列

/* AD sample */
extern  data  Union16      ad_chn_sample[13];           //最新一轮采样值（已均衡去噪声，每通道一个点，循环保存）

/* 电机控制 */                                          
extern xdata  const  Byte  Motor_Control_Code[2][12];
extern xdata  Byte         motor_index;                 //电机索引0-11
extern xdata  Byte         motor_run_mode;              //正转(1),反转(2),停止(0)
extern xdata  Uint16       motor_run_tick;              //电机转动计时tick,单位为1s
extern bdata  bit          gl_motor_adjust_flag;        //电机是否处于工作状态：0-停止工作状态;1-正处于工作状态
extern bdata  bit          gl_motor_overcur_flag;       //电机是否处于堵转状态：0-正常工作;1-电机堵转
extern bdata  bit          is_timeout;                  //电机转动时间用完：0-没有;1-时间用完

/* 传感器采样偏差 */
extern xdata  Uint16       sensor_sample_offset[13];    //传感器采样偏差：没有外力时，传感器采样值不为0，大约400左右，需要矫正。瞬间张力 = 采样值 - 采样偏差

void uart_task_init(void)
{
	Byte i;

	//uart2相关变量初始化
	recv2_state    = FSA_INIT;
	recv2_timer    = 0;
	recv2_ctr      = 0;
	recv2_chksum   = 0;

	trans2_size    = 0;
	trans2_ctr     = 0;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		uart2_send_queue[i].flag = 0; //均空闲
		uart2_recv_queue[i].flag = 0; //均空闲
	}
	uart2_q_index = 0xFF;             //无队列项进入发送流程

	//UART硬件初始化
	uart_init();                      //已经准备好串口收发，只是还未使能全局中断
}

void uart_task(void)
{
	Byte i,j;
	Byte *ptr;
	
	//1. UART2 队列处理：来自防水箱的命令包
	//找是否有等待处理的项
	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		if (uart2_recv_queue[i].flag == 1)//有等待处理的项
		{
			ptr = uart2_recv_queue[i].tdata;
			
			//处理
			switch (ptr[3])
			{
			case CMD_ZL_PRE://张力/脉冲专用命令标志
				
				switch (ptr[5])
				{
				case 0xF0://控制电机正转/反转/停止
					motor_index    = ptr[6];
					motor_run_mode = ptr[7];
					motor_run_tick = ptr[8] * 2 * 100;
                
					//控制电机的代码
					P2 = Motor_Control_Code[0][motor_index];
					P0 = Motor_Control_Code[1][motor_index];
					
					if (motor_run_mode == 1) {//电机正转
                        gl_motor_adjust_flag  = 1;
                        gl_motor_overcur_flag = 0;
						P15 = 1;
						P16 = 0;
                        is_timeout = 0;
					} else if (motor_run_mode == 2) {//电机反转
                        gl_motor_adjust_flag  = 1;
                        gl_motor_overcur_flag = 0; 
						P15 = 0;
						P16 = 1;
                        is_timeout = 0;
					} else if (motor_run_mode == 0) {//电机停止
                        gl_motor_adjust_flag  = 0;
                        gl_motor_overcur_flag = 0;
						P15 = 0;
						P16 = 0;
						P2  = 0;
						P0  = 0;
                        is_timeout = 1;
					}
					break;
					
				case 0xF1: //设置传感器采样偏差---->消除电路上的误差                  
					//1. 写入flash并更新变量
					flash_enable();
					flash_erase(EEPROM_SECTOR3);

					for (j = 0; j < 6; j++) {
						//左1 ~6
						sensor_sample_offset[j] =
						    ((Uint16)ptr[6 + (j << 1)] << 8) + ptr[7 + (j << 1)];
						flash_write(ptr[6 + (j << 1)], EEPROM_SECTOR3 + 1 + (j << 1));
						flash_write(ptr[7 + (j << 1)], EEPROM_SECTOR3 + 2 + (j << 1));
					}

					for (j = 0; j < 6; j++) {
						//右1 ~6
						sensor_sample_offset[6 + j] =
						    ((Uint16)ptr[22 + (j << 1)] << 8) + ptr[23 + (j << 1)];
						flash_write(ptr[22 + (j << 1)], EEPROM_SECTOR3 + 13 + (j << 1));
						flash_write(ptr[23 + (j << 1)], EEPROM_SECTOR3 + 14 + (j << 1));
					}

					//杆自身
                    sensor_sample_offset[12] = 0;
						
					flash_write(0x5a, EEPROM_SECTOR3);
					flash_disable(); 

					break;
				}
				
				break;
			}

			//处理完成,释放该队列项
			uart2_recv_queue[i].flag = 0;
			break;
		}
	}

	//2. UART2 队列发送：将采样结果发送给防水箱
	if (uart2_q_index == 0xFF)
	{
		//UART2无进入发送流程的队列项, 找是否有等待发送的项
		for (i = 0; i < UART_QUEUE_NUM; i++)
		{
			if (uart2_send_queue[i].flag == 1)
			{
				//有等待发送的项，安排此项发送
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
*         若返回值 >= UART_QUEUE_NUM, 则表示没有申请到空闲buffer
*----------------------------------------------------------------------------
* PURPOSE: 在串口2队列中寻找空闲队列项，若找到，返回队列项序号(0 ~ (UART_QUEUE_NUM-1))
*****************************************************************************/
Byte uart2_get_send_buffer(void)
{
	Byte i, flag;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		Disable_interrupt();
		flag = uart2_send_queue[i].flag;
		Enable_interrupt();
		if (flag == 0)   //已找到空闲Buffer
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
*         若返回值 >= UART_QUEUE_NUM, 则表示没有申请到空闲buffer
*----------------------------------------------------------------------------
* PURPOSE: 在串口2队列中寻找空闲队列项，若找到，返回队列项序号(0 ~ (UART_QUEUE_NUM-1))
*****************************************************************************/
Byte uart2_get_recv_buffer(void)
{
	Byte i, flag;

	for (i = 0; i < UART_QUEUE_NUM; i++)
	{
		Disable_interrupt();
		flag = uart2_recv_queue[i].flag;
		Enable_interrupt();
		if (flag == 0)   //已找到空闲Buffer
		{
			uart2_recv_queue[i].flag = 1;
			break;
		}
	}
	return i;
}