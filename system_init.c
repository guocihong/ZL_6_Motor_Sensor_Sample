#include "system_init.h"

/* 传感器采样偏差 */
extern xdata  Uint16       sensor_sample_offset[13];    //传感器采样偏差：没有外力时，传感器采样值不为0，大约400左右，需要矫正。瞬间张力 = 采样值 - 采样偏差
								                    
static void gpio_init(void);
static void get_config_info(void);

void system_init(void)
{   
	gpio_init();
    get_config_info();
	
	uart_task_init();
    adc_task_init();
	doorkeep_task_init();
	motor_task_init();
	
	timer0_init();
}

static void gpio_init(void)
{        
	//设置P0口为推挽输出
	P0M1 = 0x00;
	P0M0 = 0xFF;

	//设置P10~P16为推挽输出，P17为AD模式
	P1M1 = 0x80;
	P1M0 = 0x7F;

	//设置P2口为推挽输出
	P2M1 = 0x00;
	P2M0 = 0xFF;

	//设置P33为高阻输入
	P3M1 = 0x08;
	P3M0 = 0xF4;

	//设置P40,P45,P46为高阻输入
	P4M1 = 0x61;
	P4M0 = 0x00;
	P4SW = 0x70;
	
	//设置P5口为推挽输出
	P5M1 = 0x00;
	P5M0 = 0x0F;
}

static void get_config_info(void)
{
	Byte temp;
	Byte j;
	
	//使能Flash访问
	flash_enable();
	
	//读传感器采样偏差
	temp = flash_read(EEPROM_SECTOR3);
	if (temp == 0x5A) { //有有效设置
		//左1~6
		for (j = 0; j < 6; j++) {
			temp = flash_read(EEPROM_SECTOR3 + 1 + (j << 1));
			sensor_sample_offset[j] = ((Uint16)temp << 8);
			temp = flash_read(EEPROM_SECTOR3 + 2 + (j << 1));
			sensor_sample_offset[j] += temp;
		}

		//右1~6
		for (j = 0; j < 6; j++) {
			temp = flash_read(EEPROM_SECTOR3 + 13 + (j << 1));
			sensor_sample_offset[6 + j] = ((Uint16)temp << 8);
			temp = flash_read(EEPROM_SECTOR3 + 14 + (j << 1));
			sensor_sample_offset[6 + j] += temp;
		}
		
		//杆自身
		sensor_sample_offset[12] = 0;
	} else {	//取缺省值
		for (j = 0; j < 13; j++) {
			sensor_sample_offset[j] = 0;
		}
	}

	//禁止Flash访问
	flash_disable();
}