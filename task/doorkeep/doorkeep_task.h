#ifndef _DOORKEEP_TASK_H_
#define _DOORKEEP_TASK_H_

#include "config.h"

/* ÈÎÎñ×´Ì¬ */
#define DK_READ_START    0
#define DK_READ_IDLE     1
#define DK_READ_DELAY    2

void doorkeep_task_init(void);
void doorkeep_task(void);

#endif
