/***********************************************************************/
/*  FILE        :Timer.c                                               */
/*  DATE        :Mar, 2014                                             */
/*  Programmer	:xiang 'R                                              */
/*  CPU TYPE    :STM8L151G6     Crystal: 16M HSI                       */
/*  DESCRIPTION :                                                      */
/*  Mark        :ver 1.0                                               */
/***********************************************************************/
#include  <iostm8l151g4.h>				// CPU型号 
#include "Pin_define.h"		// 管脚定义
#include "initial.h"		// 初始化  预定义
#include "ram.h"		// RAM定义

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Timer 4 start   1ms
void TIM4_Init(void){				
	TIM4_PSCR = 0x06;	// Timer 4 prescaler  计数器时钟频率  f CK_CNT  =f CK_PSC  / 2的N次方
        //TIM4_PSCR = 0x08;	// Timer 4 prescaler  计数器时钟频率  f CK_CNT  = f CK_PSC/ 2(PSC[3:0])
	TIM4_ARR  = 0xF9;						// Timer 4 period
	TIM4_CR1 |= 0x01;						// Timer 4 Enable
	TIM4_IER |= 0x01;						// Timer 4 OVR interrupt
}

void TIM4_UPD_OVF(void){                             //725==1秒
    if(TIMER1s)--TIMER1s;
    if(TIMER300ms)--TIMER300ms;
    if(TIMER18ms)--TIMER18ms;
    if(TIMER250ms_STOP)--TIMER250ms_STOP;
    if (TIME_10ms)--TIME_10ms;
    else{                            // 10mS FLAG
	  TIME_10ms = 10;
	  FG_10ms = 1;
    }
	TIM4_SR1_bit.UIF=0;						// 清除中断标记
}
