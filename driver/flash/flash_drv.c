#include "driver/flash/flash_drv.h"

void flash_enable(void)
{
    IAP_CONTR = 0x80;   //����flash���ʣ��趨�ȴ�ʱ��(���22.1184MHz��Ƶ��)
}

void flash_disable(void)
{
    IAP_CMD = FLASH_CMD_STDBY;//����ģʽ,��ISP����
    IAP_CONTR = 0x00;         //��ֹflash����
    IAP_TRIG = 0x00;
}

//��ʱ21ms
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

//��ʱ2��ʱ��
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

//��ʱ55us
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