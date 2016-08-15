#ifndef _FLASH_DRV_H_
#define _FLASH_DRV_H_

#include "STC12C5A60S2.h"
#include "compiler.h"
#include <intrins.h>

//For STC12C5A32S2
/* 共29K, 58个数据扇区的起始地址 */
#define EEPROM_SECTOR1     0x0000      //每个扇区512个字节，共 58 个扇区
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

#define FLASH_CMD_STDBY   0x00         // 待机模式，无ISP操作
#define FLASH_CMD_READ    0x01         // 对数据FALSH区字节读
#define FLASH_CMD_WRITE   0x02         // 对数据FALSH区字节写
#define FLASH_CMD_ERASE   0x03         // 对数据FALSH扇区擦除

void flash_enable(void);                  // flash 访问初始设定
void flash_disable(void);                 // 禁止falsh访问
void flash_erase(Uint16 addr);            // 擦除指定地址处的扇区 
Byte flash_read(Uint16 addr);            // 读指定地址处的一个字节
void flash_write(Byte val, Uint16 addr); // 向指定地址处写入一个字节值

#endif