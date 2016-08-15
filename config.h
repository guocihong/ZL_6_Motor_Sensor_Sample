#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "STC12C5A60S2.h"
#include "compiler.h"
#include <intrins.h>
#include <string.h>
#include "stress.h"

/* Scheduler Tick */
#define SCHEDULER_TICK     5             // unit is ms

/* AD */
typedef struct strAD_Sample
{ //每点采样值
  Uint16   val;                    //当前采样值
  Uint8    index;                  //通道号，范围0 ~ 12
  Byte     valid;                  //采样数据处理标志: 0 - 已处理，可以写入新值; 1 - 新值，等待处理                                    
}sAD_Sample;

typedef struct strAD_Sum
{ //采样值累加和
  Uint16   sum;                    //累计和 (最多达64点,不会溢出)
  Uint8    point;                  //已采样点数
}sAD_Sum;

//for Uart
#define	FRAME_STX           0x16        // Frame header
#define	MAX_RecvFrame       50          // 接收缓存区大小
#define	MAX_TransFrame      50          // 发送缓存区大小
#define RECV_TIMEOUT          4           // 字节间的最大时间间隔, 单位为tick
                                   // 最小值可以为1, 如果为0则表示不进行超时判定                                    
/* state constant(仅用于接收) */
#define FSA_INIT            0            //等待帧头
#define FSA_ADDR_D          1            //等待目的地址
#define FSA_ADDR_S          2            //等待源地址
#define FSA_LENGTH          3            //等待长度字节
#define FSA_DATA            4            //等待命令串(包括 命令ID 及 参数)
#define FSA_CHKSUM          5            //等待校验和

/* Uart Queue */
typedef struct strUART_Q
{
  Byte  flag;                      //状态： 0 - 空闲； 1 - 等待发送； 2 - 正在发送; 3 - 已发送，等待应答
  Byte  tdata[MAX_TransFrame];     //数据包,不包含校验码(最后一个校验字节可以不提前计算，而在发送时边发送边计算)
  Byte  len;					   //数据包有效长度(含校验字节)
}sUART_Q;

#define UART_QUEUE_NUM      5            //UART 队列数

#define bDoorKeeper         P46          //高阻输入，门磁检测: 1-门磁闭合; 0-门磁打开（应报警）
#define bMotorOverCur       P33          //高阻输入，1-电机正常工作; 0-电机堵转
                                   //前30ms低电平不予处理，因为有毛刺干扰，随后连续20ms的低电平，表明电机堵转
/* interrupt enable */
#define Enable_interrupt()  (EA = 1)
#define Disable_interrupt() (EA = 0)

#endif