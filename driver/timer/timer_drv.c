#include "driver/timer/timer_drv.h"
#include "driver/adc/adc_drv.h"

/* UART2 */
extern idata  Byte         recv2_state;                 // receive state
extern idata  Byte         recv2_timer;                 // receive time-out, 用于字节间超时判定

/* AD sample */
extern  data  Byte         ad_index;                    //正在采样的通道号, 取值范围0~12
extern  data  sAD_Sample   ad_sample;                   //保存当前采样值

/* Doorkeep(门磁) */
extern xdata  Byte         gl_dk_tick;  	            //门磁检测计时tick

/* 电机控制 */ 
extern xdata  Byte         gl_motor_overcur_tick;       //电机堵转检测计时tick                                          
extern xdata  Uint16       motor_run_tick;              //电机转动计时tick,单位为10ms,若为0,电机停止
extern bdata  bit          gl_motor_adjust_flag;        //电机是否处于工作状态：0-停止工作状态;1-正处于工作状态
extern bdata  bit          is_timeout;                  //电机转动时间用完：0-没有;1-时间用完

/* 传感器采样偏差 */
extern xdata  Uint16       sensor_sample_offset[13];    //传感器采样偏差：没有外力时，传感器采样值不为0，大约400左右，需要矫正。瞬间张力 = 采样值 - 采样偏差

void timer0_init(void)   // 5ms@22.1184MHz
{    
    // 定时器0初始化
	AUXR &= 0x7F;		 // 设置为12T模式
	TMOD &= 0xF0;		 // 设置为工作模式1
	TMOD |= 0x01;
	TL0 = 0x00;		     // 设置定时初值
	TH0 = 0xDC;		     // 设置定时初值
	TF0 = 0;		     // 清除TF0标志
    ET0 = 1;             // 使能T0中断允许位
    IPH |= (1 << 1);
    PT0 = 0;             // 设置中断优先级为优先级2
	TR0 = 1;		     // 定时器0开始计时
	
	// 启动AD转换
	P5 = ad_index;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
	ADC_CONTR |= ADC_START;
}

void timer0_isr(void) interrupt TF0_VECTOR using 0
{	               
    // 重装初值
    TR0 = 0;
	TL0 = 0x00;		                              // 设置定时初值
	TH0 = 0xDC;		                              // 设置定时初值
	TR0 = 1;		                              // 定时器0开始计时
        
    // AD转换完成,将ADC_FLAG转换完成标志清零
    ADC_CONTR &= ~ADC_FLAG;

	// 读AD值
	if (ad_sample.valid == FALSE) {
		// 原数据已经被处理, 可以写入新数据
		ad_sample.val   = ADC_RES;                // 读高8位
		ad_sample.val   = ad_sample.val << 2;
		ad_sample.val   += (ADC_RESL & 0x03);     // 得到10bit采样值
		ad_sample.index = ad_index;
		ad_sample.valid = TRUE;
			
		//左6道钢丝和右6道钢丝的采样值需要减去采样偏差以及杆自身的采样值需要减去采样偏差
        if ((ad_index >= 0) && (ad_index <= 11)) {
            if (ad_sample.val > sensor_sample_offset[ad_index]) {
                ad_sample.val -= sensor_sample_offset[ad_index];
            } else {
                ad_sample.val = 0;
            }
        }
        
		// 启动下一通道采样
		if (ad_index >= 12) {
			ad_index = 0;
		} else {
			ad_index++;
		}

		P5 = ad_index;	                          // 选择模拟输入
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
   		ADC_CONTR = ADC_POWER_ENABLE | ADC_SPEED_540 | ADC_SAMPLE_CHANNEL; // 启动转换
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        ADC_CONTR |=  ADC_START;
	}

	// increment task tick counters
	gl_dk_tick++;                                 //门磁检测计时tick
    if (gl_motor_overcur_tick > 0) {
    	gl_motor_overcur_tick--;                      //电机堵转计时tick
    }
	 
	if (motor_run_tick > 0) {
		motor_run_tick--;
		
		if (motor_run_tick == 0) {                //时间到,停止电机
			P15 = 0;
			P16 = 0;
			P2  = 0;
			P0  = 0;
            gl_motor_adjust_flag = 0;
            is_timeout = 1;
		}
	}
	
	// UART2字节之间接收超时
	if (recv2_state != FSA_INIT) { 
		//非初始状态，需要检测是否超时
		if (recv2_timer > 0) {
			recv2_timer--;
		}
		
		if (recv2_timer == 0) {
			recv2_state = FSA_INIT;               //接收超时, 恢复至初始状态			
		}
	}
}