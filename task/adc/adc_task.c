#include "task/adc/adc_task.h"

/* AD sample */
#define AD_EQU_PNUM             4                                 //每道钢丝采样4次求平均值

extern idata  Byte         ad_index;                    //正在采样的通道号, 取值范围0~12
extern  data  sAD_Sample   ad_sample;                   //保存当前采样值
extern idata  sAD_Sum      ad_samp_equ[13];             //均衡去嘈声求和
extern xdata  Union16      ad_chn_sample[13];           //最新一轮采样值（已均衡去噪声，每通道一个点，循环保存）

/* UART2 */
extern  data  Byte         uart2_q_index;               // 正在发送某队列项的序号：若为0xFF, 表示没有任何项进入发送流程
extern xdata  sUART_Q      uart2_send_queue[UART_QUEUE_NUM];     // 串口发送队列

/* Doorkeep(门磁) */
extern bdata  bit          gl_dk_status;                //门磁开关状态（每1s动态检测）: 1 - 闭合; 0 - 打开(需要报警)                    

/* 电机控制 */
extern bdata  bit          gl_motor_overcur_flag;       //电机是否处于堵转状态：0-正常工作;1-电机堵转
extern bdata  bit          gl_motor_adjust_flag;        //电机是否处于工作状态：0-停止工作状态;1-正处于工作状态
extern bdata  bit          is_timeout;                  //电机转动时间用完：0-没有;1-时间用完

extern Byte uart2_get_send_buffer(void);

void adc_task_init(void)
{
	Byte i;

	//相关变量初始化
	ad_index        = 0;
	ad_sample.valid = 0;                     //空闲，可以写入新值
	for (i = 0; i < 13; i++)
	{
		ad_samp_equ[i].sum       = 0;        //均衡去嘈声求和
		ad_samp_equ[i].point     = 0;
		ad_chn_sample[i].w       = 0;        //最新一轮采样值
	}

    is_timeout = 0;
    
	//adc硬件初始化
	adc_init();
}

void adc_task(void)
{
	Byte    i,j;
	Byte    index;          //采样通道号
	Uint16  val_temp;       //新送入的10bit采样值,  后作临时变量
	Uint16  val;            //4点均衡后得到的平均采样值, 作为一个可进行超限判断的最小点

	if (ad_sample.valid)    //有新采样数据到达
	{
		// 0. 保存到临时变量
		val_temp = ad_sample.val;
		index    = ad_sample.index;

		// 1. 保存到均衡去嘈声求和中
		ad_samp_equ[index].sum += val_temp;
		ad_samp_equ[index].point++;

		// 2. 当前通道是否满足去嘈声点数
		if (ad_samp_equ[index].point == AD_EQU_PNUM)
		{
			// 已满去嘈声点数，可求出均衡后的一个点
			// 2.a 求出对应通道的一个采样点
			val = ad_samp_equ[index].sum >> 2;  //除于4

			// 2.b 清零当前通道的去嘈声求和结构
			ad_samp_equ[index].sum = 0;
			ad_samp_equ[index].point = 0;

			// 2.c 保存实时采样值
			ad_chn_sample[index].w = val;   //保存到最新一轮采样值数组中

			// 2.d 实时发送采样值
			if (index == 12)
			{ //已经完成13个通道的一次完整采样，可发送采样值
				i = uart2_get_send_buffer();
				if (i < UART_QUEUE_NUM)
				{ //找到了空闲buffer, 写入data
					uart2_send_queue[i].tdata[0] = FRAME_STX;
					uart2_send_queue[i].tdata[1] = 0xFF;
					uart2_send_queue[i].tdata[2] = 0x00;
					
					uart2_send_queue[i].tdata[3] = 0x21;
					uart2_send_queue[i].tdata[4] = 0xE8;
					uart2_send_queue[i].tdata[5] = 0x1F;
					uart2_send_queue[i].tdata[6] = 0x1C;

					//左ch1 ~ ch6
					for (j = 0; j < 6; j++)
					{
						uart2_send_queue[i].tdata[7 + (j << 1)] = HIGH(ad_chn_sample[j].w);
						uart2_send_queue[i].tdata[8 + (j << 1)] = LOW(ad_chn_sample[j].w);
					}
					
					//右ch1 ~ ch6
					for (j = 0; j < 6; j++)
					{
						uart2_send_queue[i].tdata[19 + (j << 1)] = HIGH(ad_chn_sample[6 + j].w);
						uart2_send_queue[i].tdata[20 + (j << 1)] = LOW(ad_chn_sample[6 + j].w);
					}

					//杆自身
					uart2_send_queue[i].tdata[31] = HIGH(ad_chn_sample[12].w);
					uart2_send_queue[i].tdata[32] = LOW(ad_chn_sample[12].w);

					//门磁状态
					uart2_send_queue[i].tdata[33] = (Byte)gl_dk_status;
					
					//电机运转状态
					uart2_send_queue[i].tdata[34] = (Byte)gl_motor_adjust_flag;
					
					//电机堵转状态
					uart2_send_queue[i].tdata[35] = (Byte)gl_motor_overcur_flag;
					
                    //时间是否用完
                    uart2_send_queue[i].tdata[36] = (Byte)is_timeout;

					uart2_send_queue[i].len       = 38;
                    
                    gl_motor_overcur_flag         = 0;
                    is_timeout                    = 0;
				}
				else
				{   //无空闲buffer, 丢弃本命令
					//检查: 若有队列项正在发送, 等待它完成
					while (uart2_q_index != 0xFF);	//若死锁,将引起 WDT 复位
				}
			}
		}

		//3.当前采样值处理完毕，允许新的采样值输入
		ad_sample.valid = FALSE;
	}
}