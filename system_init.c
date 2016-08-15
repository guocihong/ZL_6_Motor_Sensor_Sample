#include "system_init.h"

/* ����������ƫ�� */
extern xdata  Uint16       sensor_sample_offset[13];    //����������ƫ�û������ʱ������������ֵ��Ϊ0����Լ400���ң���Ҫ������˲������ = ����ֵ - ����ƫ��
								                    
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
	//����P0��Ϊ�������
	P0M1 = 0x00;
	P0M0 = 0xFF;

	//����P10~P16Ϊ���������P17ΪADģʽ
	P1M1 = 0x80;
	P1M0 = 0x7F;

	//����P2��Ϊ�������
	P2M1 = 0x00;
	P2M0 = 0xFF;

	//����P33Ϊ��������
	P3M1 = 0x08;
	P3M0 = 0xF4;

	//����P40,P45,P46Ϊ��������
	P4M1 = 0x61;
	P4M0 = 0x00;
	P4SW = 0x70;
	
	//����P5��Ϊ�������
	P5M1 = 0x00;
	P5M0 = 0x0F;
}

static void get_config_info(void)
{
	Byte temp;
	Byte j;
	
	//ʹ��Flash����
	flash_enable();
	
	//������������ƫ��
	temp = flash_read(EEPROM_SECTOR3);
	if (temp == 0x5A) { //����Ч����
		//��1~6
		for (j = 0; j < 6; j++) {
			temp = flash_read(EEPROM_SECTOR3 + 1 + (j << 1));
			sensor_sample_offset[j] = ((Uint16)temp << 8);
			temp = flash_read(EEPROM_SECTOR3 + 2 + (j << 1));
			sensor_sample_offset[j] += temp;
		}

		//��1~6
		for (j = 0; j < 6; j++) {
			temp = flash_read(EEPROM_SECTOR3 + 13 + (j << 1));
			sensor_sample_offset[6 + j] = ((Uint16)temp << 8);
			temp = flash_read(EEPROM_SECTOR3 + 14 + (j << 1));
			sensor_sample_offset[6 + j] += temp;
		}
		
		//������
		sensor_sample_offset[12] = 0;
	} else {	//ȡȱʡֵ
		for (j = 0; j < 13; j++) {
			sensor_sample_offset[j] = 0;
		}
	}

	//��ֹFlash����
	flash_disable();
}