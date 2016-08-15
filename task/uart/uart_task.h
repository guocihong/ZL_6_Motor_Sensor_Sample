#ifndef _UART_TASK_H_
#define _UART_TASK_H_

#include "driver/uart/uart_drv.h"
#include "driver/flash/flash_drv.h"
#include "config.h"

/* ϵͳ����� */
#define CMD_ADDR_DEF     0x30    //ȱʡ������ַ
#define CMD_ADDR_BC      255     //�㲥��ַ
#define CMD_ADDR_UNSOLV  254     //δ��¼��δ��ȷ���õĵ�ַ

//˫������ַģ��
#define CMD_DADDR_qSTAT  0xE0    //ѯ�ʷ���״̬
#define CMD_DADDR_qPARA  0xE1    //ѯ�ʵ�ַ

#define CMD_ACK_OK       0x00    
#define CMD_DADDR_aSTAT  0xF0    //Ӧ�� - ��������
#define CMD_DADDR_aPARA  0xF1    //Ӧ�� - ����ѯ��/����

//����/����MODģ��
#define CMD_ZL_PRE       0xE8    //����/����ר�������־

void uart_task_init(void);
void uart_task(void);

Byte uart2_get_send_buffer(void);
Byte uart2_get_recv_buffer(void);

Byte uart3_get_send_buffer(void);
Byte uart3_get_recv_buffer(void);

Byte uart4_get_send_buffer(void);
Byte uart4_get_recv_buffer(void);
#endif