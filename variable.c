#include "config.h"

/* UART2 */	
xdata  Byte         recv2_buf[MAX_RecvFrame];    // receiving buffer
idata  Byte         recv2_state;                 // receive state
idata  Byte         recv2_timer;                 // receive time-out, 用于字节间超时判定
idata  Byte         recv2_chksum;                // computed checksum
idata  Byte         recv2_ctr;                   // reveiving pointer
                    
xdata  Byte         trans2_buf[MAX_TransFrame];  // uart transfer message buffer
idata  Byte         trans2_ctr;                  // transfer pointer
idata  Byte         trans2_size;                 // transfer bytes number
idata  Byte         trans2_chksum;               // computed check-sum of already transfered message

 data  Byte         uart2_q_index;               // 正在发送某队列项的序号：若为0xFF, 表示没有任何项进入发送流程
xdata  sUART_Q      uart2_send_queue[UART_QUEUE_NUM];     // 串口队列
xdata  sUART_Q      uart2_recv_queue[UART_QUEUE_NUM];     // 串口队列

/* AD sample */
 data  Byte         ad_index;                    //正在采样的通道号, 取值范围0~12
 data  sAD_Sample   ad_sample;                   //保存当前采样值
 data  sAD_Sum      ad_samp_equ[13];             //均衡去嘈声求和
 data  Union16      ad_chn_sample[13];           //最新一轮采样值（已均衡去噪声，每通道一个点，循环保存）
 data  Byte         ad_equ_pum;                  //每道钢丝采样多次求平均值
 
/* Doorkeep(门磁) */
bdata  bit          gl_dk_status;                //门磁开关状态（每1s动态检测）: 1 - 闭合; 0 - 打开(需要报警)                    
xdata  Byte         gl_dk_tick;  	             //门磁检测计时tick

/* 电机控制 */
xdata  Byte         gl_motor_overcur_tick;       //电机堵转检测计时tick
bdata  bit          gl_motor_overcur_flag;       //电机是否处于堵转状态：0-正常工作;1-电机堵转
bdata  bit          gl_motor_adjust_flag;        //电机是否处于工作状态：0-停止工作状态;1-正处于工作状态

                                                 //左1 右1 左2 右2 左3 右3 左4 右4 左5 右5 左6 右6
xdata  const  Byte  Motor_Control_Code[2][12] = {{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x00},
												 {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x04,0x08}};
												 
xdata  Byte         motor_index;                 //电机索引0-11
xdata  Byte         motor_run_mode;              //正转(1),反转(2),停止(0)
xdata  Uint16       motor_run_tick;              //电机转动计时tick,单位为1s
bdata  bit          is_timeout;                  //电机转动时间用完：0-没有;1-时间用完
                                                 
/* 传感器采样偏差 */
xdata  Uint16      sensor_sample_offset[13];     //传感器采样偏差：没有外力时，传感器采样值不为0，大约400左右，需要矫正。瞬间张力 = 采样值 - 采样偏差
									             //左1 右1 左2 右2 ... 左6 右6 杆自身
