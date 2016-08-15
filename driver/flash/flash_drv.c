#include "driver/flash/flash_drv.h"

void flash_enable(void)
{
    IAP_CONTR = 0x80;   //允许flash访问，设定等待时间(针对22.1184MHz主频率)
}

void flash_disable(void)
{
    IAP_CMD = FLASH_CMD_STDBY;//待机模式,无ISP操作
    IAP_CONTR = 0x00;         //禁止flash访问
    IAP_TRIG = 0x00;
}

//耗时21ms
void flash_erase(Uint16 addr)
{
    IAP_CMD = FLASH_CMD_ERASE;
    IAP_ADDRH = HIGH(addr);
    IAP_ADDRL = LOW(addr);
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    _nop_();
    _nop_();
}

//耗时2个时钟
Byte flash_read(Uint16 addr)
{
    Byte dat;
    IAP_CMD = FLASH_CMD_READ;
    IAP_ADDRH = HIGH(addr);
    IAP_ADDRL = LOW(addr);
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    _nop_();
    _nop_();
    dat = IAP_DATA;
    
    return dat;
}

//耗时55us
void flash_write(Byte val, Uint16 addr)
{
    IAP_CMD = FLASH_CMD_WRITE;
    IAP_ADDRH = HIGH(addr);
    IAP_ADDRL = LOW(addr);
    IAP_DATA = val;
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    _nop_();
    _nop_();
}