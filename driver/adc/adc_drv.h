#ifndef _ADC_DRV_H_
#define _ADC_DRV_H_

#include "STC12C5A60S2.h"
#include "compiler.h"
#include <intrins.h>

#define ADC_POWER_ENABLE    (1 << 7)
#define ADC_SPEED_90        (3 << 5)
#define ADC_SPEED_180       (2 << 5)
#define ADC_SPEED_360       (1 << 5)
#define ADC_SPEED_540       (0 << 5)
#define ADC_FLAG            (1 << 4)
#define ADC_START           (1 << 3)
#define ADC_SAMPLE_CHANNEL  (7 << 0)

void adc_init(void);

#endif