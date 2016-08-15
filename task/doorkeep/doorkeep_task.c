#include "task/doorkeep/doorkeep_task.h"

#define DK_CHECK_INTERVAL       (1000 / SCHEDULER_TICK)	          //门磁开关检查周期
#define DK_CONFORM_DELAY        ( 100 / SCHEDULER_TICK)            //门磁开关变化确认延时

/* Doorkeep(门磁) */
extern bdata  bit          gl_dk_status;                //门磁开关状态（每1s动态检测）: 1 - 闭合; 0 - 打开(需要报警)                    
extern xdata  Byte         gl_dk_tick;  	            //门磁检测计时tick

static xdata  Byte         dk_read_state;               //task state

void doorkeep_task_init(void)
{
	gl_dk_tick    = 0;
	gl_dk_status  = 1;                                   //上电时，缺省为闭合
	dk_read_state = DK_READ_START;
}

void doorkeep_task(void)
{
	switch (dk_read_state)
	{
	case DK_READ_START: //开始任务
		dk_read_state = DK_READ_IDLE;
		break;

	case DK_READ_IDLE://检测门磁有无变化
		if (gl_dk_tick > DK_CHECK_INTERVAL) {
			//每秒检测一次
			gl_dk_tick = 0;
			if (gl_dk_status == 1) {
				//原来为门磁闭合
				if (bDoorKeeper == 0) {
					//门磁可能被打开, 进入延时确认阶段
					dk_read_state = DK_READ_DELAY;
				}
			} else {
				//原来为门磁打开
				if (bDoorKeeper == 1) {
					//门磁可能被闭合, 进入延时确认阶段
					dk_read_state = DK_READ_DELAY;
				}
			}
		}
		break;

	case DK_READ_DELAY://延时确认
		if (gl_dk_tick > DK_CONFORM_DELAY) {
			// 延时时间到, 判断是否是稳定的变化
			if ((gl_dk_status == 1) && (bDoorKeeper == 0)) {// 门磁已经打开				
				//更新变量
				gl_dk_status = 0;
			} else if ((gl_dk_status == 0) && (bDoorKeeper == 1)) {// 门磁已经闭合
				//更新变量
				gl_dk_status = 1;
			}

			gl_dk_tick = 0;
			dk_read_state = DK_READ_IDLE;
		}
		break;
	}
}
