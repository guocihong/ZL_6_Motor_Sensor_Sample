#ifndef _UART_TASK_H_
#define _UART_TASK_H_

#include "driver/uart/uart_drv.h"
#include "driver/flash/flash_drv.h"
#include "config.h"

/* 系统命令定义 */
#define CMD_ADDR_DEF     0x30    //缺省主机地址
#define CMD_ADDR_BC      255     //广播地址
#define CMD_ADDR_UNSOLV  254     //未烧录或未正确设置的地址

//双防区地址模块
#define CMD_DADDR_qSTAT  0xE0    //询问防区状态
#define CMD_DADDR_qPARA  0xE1    //询问地址

#define CMD_ACK_OK       0x00    
#define CMD_DADDR_aSTAT  0xF0    //应答 - 防区报警
#define CMD_DADDR_aPARA  0xF1    //应答 - 参数询问/设置

//张力/脉冲MOD模块
#define CMD_ZL_PRE       0xE8    //张力/脉冲专用命令标志

void uart_task_init(void);
void uart_task(void);

Byte uart2_get_send_buffer(void);
Byte uart2_get_recv_buffer(void);

Byte uart3_get_send_buffer(void);
Byte uart3_get_recv_buffer(void);

Byte uart4_get_send_buffer(void);
Byte uart4_get_recv_buffer(void);
#endif