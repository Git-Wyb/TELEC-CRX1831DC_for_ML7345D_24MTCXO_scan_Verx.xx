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
#include <iostm8l151g4.h> // CPU型号
#include "Pin_define.h"   // 管脚定义
#include "initial.h"      // 初始化  预定义
#include "ram.h"          // RAM定义
#include "ADF7030_1.h"    // 初始化ADF7021
#include "Timer.h"        // 定时器
#include "ID_Decode.h"    // ID_Decode处理
#include "eeprom.h"       // eeprom
#include "uart.h"         // uart
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
    _DI();             // 关全局中断
    RAM_clean();       // 清除RAM
    WDT_init();        //看门狗
    VHF_GPIO_INIT();   //IO初始化
    SysClock_Init();   //系统时钟初始化
    InitialFlashReg(); //flash EEPROM
    eeprom_sys_load(); //ID载入
    TIM4_Init();       // 定时器
    beep_init();       // 蜂鸣器
    ClearWDT();        // Service the WDT
    
    PROFILE_CH_FREQ_32bit_200002EC = 426075000;
    PROFILE_RADIO_AFC_CFG1_32bit_2000031C = 0x0005005A;  
    PROFILE_RADIO_DATA_RATE_32bit_200002FC = 0x6400000C;
    PROFILE_GENERIC_PKT_FRAME_CFG1_32bit_20000500 = 0x0000100C;    
    ADF7030Init();     //射频初始化
    
    UART1_INIT();      // UART1 for PC Software
    _EI();             // 允许中断
    TIME_power_led=500;
    ClearWDT();        // Service the WDT
    RF_test_mode();
    FLAG_APP_RX = 1;
    FG_Receiver_LED_RX = 0;
    TIME_EMC = 10;
    FLAG_testNo91=0;
	FLAG_testBEEP=0;
    while (1)
    {
        ClearWDT(); // Service the WDT
        if(FLAG_testBEEP!=0)TEST_beep();

        if (time_Login_exit_256 == 0)
            ID_Decode_OUT();
        ID_learn();
		APP_TX_PACKET();
        if(FLAG_APP_RX==1)
        {
    		  Freq_Scanning();
    		  //if(Scan_step==2)
			  	SCAN_RECEIVE_PACKET(); //ɨ���������
        }
        TranmissionACK();
        //        READ_RSSI_avg();

        if (FG_Receiver_LED_RX == 1)
            Receiver_LED_RX = 1;
        else if (FG_Receiver_LED_RX == 0)
            Receiver_LED_RX = 0;
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
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
