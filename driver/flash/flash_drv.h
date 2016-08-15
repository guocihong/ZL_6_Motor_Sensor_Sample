#ifndef _FLASH_DRV_H_
#define _FLASH_DRV_H_

#include "STC12C5A60S2.h"
#include "compiler.h"
#include <intrins.h>

//For STC12C5A32S2
/* ��29K, 58��������������ʼ��ַ */
#define EEPROM_SECTOR1     0x0000      //ÿ������512���ֽڣ��� 58 ������
#define EEPROM_SECTOR2     0x0200
#define EEPROM_SECTOR3     0x0400
#define EEPROM_SECTOR4     0x0600
#define EEPROM_SECTOR5     0x0800
#define EEPROM_SECTOR6     0x0A00
#define EEPROM_SECTOR7     0x0C00
#define EEPROM_SECTOR8     0x0E00
#define EEPROM_SECTOR9     0x1000
#define EEPROM_SECTOR10    0x1200
#define EEPROM_SECTOR11    0x1400
#define EEPROM_SECTOR12    0x1600
#define EEPROM_SECTOR13    0x1800
#define EEPROM_SECTOR14    0x1A00
#define EEPROM_SECTOR15    0x1C00
#define EEPROM_SECTOR16    0x1E00

#define FLASH_CMD_STDBY   0x00         // ����ģʽ����ISP����
#define FLASH_CMD_READ    0x01         // ������FALSH���ֽڶ�
#define FLASH_CMD_WRITE   0x02         // ������FALSH���ֽ�д
#define FLASH_CMD_ERASE   0x03         // ������FALSH��������

void flash_enable(void);                  // flash ���ʳ�ʼ�趨
void flash_disable(void);                 // ��ֹfalsh����
void flash_erase(Uint16 addr);            // ����ָ����ַ�������� 
Byte flash_read(Uint16 addr);            // ��ָ����ַ����һ���ֽ�
void flash_write(Byte val, Uint16 addr); // ��ָ����ַ��д��һ���ֽ�ֵ

#endif