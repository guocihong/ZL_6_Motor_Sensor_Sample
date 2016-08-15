#include "system_init.h"

/*
注意事项：
1、需要设置采样值清零，不需要设置出厂值
2、测试过程中需要与防水箱电路板配合
3、测试事项：
	a、测试13路传感器是否正常
	b、测试12路电机是否正常
	c、测试通信串口是否正常
	d、测试门磁是否正常
	e、测试设置采样值清零是否正常
*/

void main(void)
{
    //系统初始化
    system_init();

    //打开总中断
    Enable_interrupt();

    //使用看门狗
    Wdt_enable();// 2.276s 

    while(1) {
		//处理AD采集的结果
        adc_task();
		
        //解析uart接收的命令
        uart_task();

        //门磁处理
        doorkeep_task();

        //检测电机是否堵转
        motor_task();

        //喂狗
        Wdt_refresh();
    }
}