/**
  ******************************************************************************
  * @file    Project/STM8L15x_StdPeriph_Template/main.c
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    13-May-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */  
	
/* Includes ------------------------------------------------------------------*/
#include  <iostm8l151g4.h>				// CPU型号
//#include "stm8l15x.h"
#include "Pin_define.h"		// 管脚定义
#include "initial.h"		// 初始化  预定义
#include "ram.h"		// RAM定义
#include "adf7021.h"		// 初始化ADF7021
#include "Timer.h"		// 定时器
#include "ID_Decode.h"
#include "eeprom.h"		// eeprom
#include "uart.h"		// uart
/** @addtogroup STM8L15x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

void main(void)
{    
    _DI();		// 关全局中断
    RAM_clean();       // 清除RAM 
    WDT_init();
    VHF_GPIO_INIT();
    SysClock_Init();  
    InitialFlashReg();
    eeprom_sys_load();
    EXIT_init();
    TIM4_Init();
    UART1_INIT();  // UART1 for PC Software 
    _EI();       // 允许中断
    beep_init();
    dd_set_ADF7021_Power_on();
    
    RF_test_mode();
    
    FLAG_APP_RX=1;
    dd_set_RX_mode();
    TIME_EMC=10;
    //dd_set_TX_mode();
  while (1)
  {
    ClearWDT(); // Service the WDT
    ID_Decode_IDCheck();
    if(time_Login_exit_256==0)ID_Decode_OUT();
    Freq_Scanning();
    ID_learn();
    READ_RSSI_avg();
    
    if((RAM_rssi_AVG>=60)||(FG_Receiver_LED_RX==1))Receiver_LED_RX=1;   //26   35
    else if((RAM_rssi_AVG<=59)&&(FG_Receiver_LED_RX==0))Receiver_LED_RX=0;  //25  34
             
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
