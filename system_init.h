#ifndef _SYSTEM_INIT_H_
#define _SYSTEM_INIT_H_

#include "task/uart/uart_task.h"
#include "task/adc/adc_task.h"
#include "task/doorkeep/doorkeep_task.h"
#include "task/motor/motor_task.h"

#include "driver/timer/timer_drv.h"
#include "driver/wdt/wdt_drv.h"

void system_init(void);

#endif