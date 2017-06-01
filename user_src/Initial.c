/***********************************************************************/
/*  FILE        :initial.c                                             */
/*  DATE        :Mar, 2013                                             */
/*  Programmer	:xiang 'R                                              */
/*  CPU TYPE    :STM8L151G6     Crystal: 16M HSI                       */
/*  Mark        :ver 1.0                                               */
/***********************************************************************/
#include <iostm8l151g4.h>
//#include        "stm8l15x.h"
#include "Pin_define.h" // 管脚定义
#include "initial.h"    // 初始化  预定义
#include "ram.h"        // RAM定义
#include "ADF7030_1.h"
#include "uart.h" // uart
#include "Timer.h"
uFLAG YellowLedFlag, RedLedFalg;
void RAM_clean(void)
{ // 清除RAM
    //  asm("ldw X,#0");
    //  asm("clear_ram1.l");
    //  asm("clr (X)");
    //  asm("incw X");
    //  asm("cpw X,#0x6ff");
    //  asm("jrule clear_ram1");
}
void WDT_init(void)
{
    IWDG_KR = 0xCC;
    IWDG_KR = 0x55;
    IWDG_PR = 3;
    IWDG_KR = 0xAA;
}
void ClearWDT(void)
{
    IWDG_KR = 0xAA;
}
//========================GPIO说明===============================================================
//  GPIO_Mode_In_FL_No_IT      = (uint8_t)0x00,   /*!< Input floating, no external interrupt */
//  GPIO_Mode_In_PU_No_IT      = (uint8_t)0x40,   /*!< Input pull-up, no external interrupt */
//  GPIO_Mode_In_FL_IT         = (uint8_t)0x20,   /*!< Input floating, external interrupt */
//  GPIO_Mode_In_PU_IT         = (uint8_t)0x60,   /*!< Input pull-up, external interrupt */
//  GPIO_Mode_Out_OD_Low_Fast  = (uint8_t)0xA0,   /*!< Output open-drain, low level, 10MHz */
//  GPIO_Mode_Out_PP_Low_Fast  = (uint8_t)0xE0,   /*!< Output push-pull, low level, 10MHz */
//  GPIO_Mode_Out_OD_Low_Slow  = (uint8_t)0x80,   /*!< Output open-drain, low level, 2MHz */
//  GPIO_Mode_Out_PP_Low_Slow  = (uint8_t)0xC0,   /*!< Output push-pull, low level, 2MHz */
//  GPIO_Mode_Out_OD_HiZ_Fast  = (uint8_t)0xB0,   /*!< Output open-drain, high-impedance level, 10MHz */
//  GPIO_Mode_Out_PP_High_Fast = (uint8_t)0xF0,   /*!< Output push-pull, high level, 10MHz */
//  GPIO_Mode_Out_OD_HiZ_Slow  = (uint8_t)0x90,   /*!< Output open-drain, high-impedance level, 2MHz */
//  GPIO_Mode_Out_PP_High_Slow = (uint8_t)0xD0    /*!< Output push-pull, high level, 2MHz */
//===降低功耗说明：①I/O没用，必须置Input pull-up    ②I/O外围有IC，但没用，必须置Input floating=====

void VHF_GPIO_INIT(void)
{ // CPU端口设置
    /****************端口设置说明***************************
CR1寄存器  输出 Output（1=推挽、0=OC）
           输入 Input（1=上拉、0=浮动）
***************end************************************/
    //    HA_GPIO_Init();
    KEY_GPIO_Init();             // 输入 test脚 登录键
    Receiver_vent_direc = Input; // Input   受信机换气联动ON/OFF
    Receiver_vent_CR1 = 1;

    PIN_BEEP_direc = Output; // Output   蜂鸣器
    PIN_BEEP_CR1 = 1;
    PIN_BEEP = 0;

    LED_GPIO_Init();
    Inverters_OUT_direc = Input; // 输入   继电器输出信号反向   低电平有效
    Inverters_OUT_CR1 = 1;
    Receiver_OUT_GPIO_Init(); // Output   受信机继电器
}
//============================================================================================
void SysClock_Init(void)
{ // 系统时钟（外部时钟）
    //    /* Infinite loop */
    //    CLK_DeInit();                                         //时钟恢复默认
    //    CLK_HSICmd(ENABLE);
    //    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY)==RESET);//等待直到LSI稳定
    ////    CLK_HSEConfig(CLK_HSE_ON);
    ////    CLK_HSEConfig(CLK_HSE_ON);
    ////    while(CLK_GetFlagStatus(CLK_FLAG_HSERDY)==RESET);//等待直到HSE稳定
    //    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    ////    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);
    //    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
    //    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    //    CLK_SYSCLKSourceSwitchCmd(DISABLE);
    ////    CLK_LSICmd(ENABLE);
    ////    while(CLK_GetFlagStatus(CLK_FLAG_LSIRDY)==RESET);//等待直到LSI稳定
    ////    CLK_HSEConfig(CLK_HSE_OFF);
    ////    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_LSI);
    ////    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    ////    CLK_SYSCLKSourceSwitchCmd(DISABLE);
    //
    //    //CLK_LSICmd(ENABLE);   //使能LSI时钟给看门狗用
    //    //while(CLK_GetFlagStatus(CLK_FLAG_LSIRDY)==RESET);//等待直到LSI稳定

    CLK_ICKCR_HSION = 1; // 使能内部RC OSC（16.00MHz）
    while ((CLK_ICKCR & 0x02) == 0)
        ;              // 检查内部晶振
    CLK_SWR = 0x01;    // 指定HSI为主时钟
                       //	while(( CLK_SWCR & 0x08 ) == 0 );		// 等待HSI切换
    CLK_SWCR_SWEN = 1; // 执行切换
    CLK_CKDIVR = 0x00; // 设置时钟分频  f HSI= f HSI RC输出/1    f CPU= f MASTER
    //---------------------------------------- 外设
    //CLK_PCKENR1 = 0x84;						// T1,UART1
    CLK_PCKENR1 = 0x64; // T4,UART1,beep
    CLK_PCKENR2 = 0x03; // ADC,T1

    CLK_ICKCR_LSION = 1; // 使能内部LSI OSC（38KHz）
    while (CLK_ICKCR_LSIRDY == 0)
        ; // 检查内部LSI OSC
}

void beep_init(void)
{
    //BEEP_CSR=0x4E;
    BEEP_CSR2 = 0;
    BEEP_CSR2_BEEPDIV = 3;
    BEEP_CSR2_BEEPSEL = 1;
    CLK_CBEEPR_CLKBEEPSEL0 = 1;
    CLK_CBEEPR_CLKBEEPSEL1 = 0;
}

//===================Delayus()延时===============//    Crystal: 16M HSI
void Delayus(unsigned char timer)
{
    unsigned char x; //延时T=((timer-1)*0.313+2 us
    for (x = 0; x < timer; x++)
        __asm("nop");
}
void HA_GPIO_Init(void)
{
    //    HA_L_signal_direc = Input; // Input   HA 下限信号   低电平有效
    //    HA_L_signal_CR1 = 1;

    HA_ERR_signal_direc = Input; // Input   HA 异常信号  低电平有效
    HA_ERR_signal_CR1 = 1;

    HA_Sensor_signal_direc = Input; // Input   HA 传感器信号  低电平有效
    HA_Sensor_signal_CR1 = 1;
}
void Receiver_OUT_GPIO_Init(void)
{
    if (Inverters_OUT == 1)
    {
        FG_allow_out = 1;
        FG_NOT_allow_out = 0;
    }
    else
    {
        FG_allow_out = 0;
        FG_NOT_allow_out = 1;
    }

    Receiver_OUT_OPEN_direc = Output; // Output   受信机继电器OPEN  高电平有效
    Receiver_OUT_OPEN_CR1 = 1;
    Receiver_OUT_OPEN = FG_NOT_allow_out;

    Receiver_OUT_CLOSE_direc = Output; // Output   受信机继电器CLOSE  高电平有效
    Receiver_OUT_CLOSE_CR1 = 1;
    Receiver_OUT_CLOSE = FG_NOT_allow_out;

    Receiver_OUT_STOP_direc = Output; // Output   受信机继电器STOP  高电平有效
    Receiver_OUT_STOP_CR1 = 1;
    Receiver_OUT_STOP = FG_NOT_allow_out;

    Receiver_OUT_VENT_direc = Output;
    Receiver_OUT_VENT_CR1 = 1;
    Receiver_OUT_VENT = FG_NOT_allow_out;
}
/**
****************************************************************************
* @Function	: void LED_GPIO_Init(void)
* @file		: Initial.c
* @Author	: Xiaowine
* @date		: 2017/4/10
* @version	: V1.0
* @brief
**/
void LED_GPIO_Init(void)
{
    // LED_YELLOW_DDR = Output; /* 设置数据方向寄存器 1为输出，0为输入--查看STM8寄存器.pdf P87 */
    // LED_YELLOW_CR1 = 1;      /* 设置推挽输出--查看STM8寄存器RM0031.pdf 10.9*/
    // LED_YELLOW_CR2 = 1;      /* 设置输出频率 1为10M，0为2M--查看STM8寄存器.pdf P89 */

    // LED_RED_DDR = Output; /* 设置数据方向寄存器 1为输出，0为输入--查看STM8寄存器.pdf P87 */
    // LED_RED_CR1 = 1;      /* 设置推挽输出--查看STM8寄存器RM0031.pdf 10.9*/
    // LED_RED_CR2 = 1;      /* 设置输出频率 1为10M，0为2M--查看STM8寄存器.pdf P89 */

    //    Receiver_LED_OUT_direc = Output; // Output   受信机继电器动作输出  高电平有效
    //    Receiver_LED_OUT_CR1 = 1;
    //    Receiver_LED_OUT = 1;

    Receiver_LED_TX_direc = Output; // Output   受信机送信指示  高电平有效
    Receiver_LED_TX_CR1 = 1;
    Receiver_LED_TX = 0;

    Receiver_LED_RX_direc = Output; // Output   受信机受信指示  高电平有效
    Receiver_LED_RX_CR1 = 1;
    Receiver_LED_RX = 0;
}
/**
 ****************************************************************************
 * @Function : void LEDCtr(void)
 * @File     : Initial.c
 * @Program  :
 * @Created  : 2017/5/8 by Xiaowine
 * @Brief    :
 * @Version  : V1.0
**/
void LEDCtr(void)
{
    switch (YellowStutue & 0x0f)
    {
    case LEDOFFFLAG:
        YELLOWLED_OFF();
        break;
    case LEDONFLAG:
        LED_YELLOW = LED_ON;
        break;
    case LEDFLASHASECONDFLAG:
    {
        if (YellowStutue & 0x80)
        {
            YELLOWLED_FLASH_SECOND();
            YellowStutue &= 0x7F;
        }
        if (LedYELLOWTimer == 1)
        {
            YellowStutue &= 0x3F;
            YELLOWLED_OFF();
        }
    }
    break;
    case LEDFLASHFLAG:
    {
        if (YellowStutue & 0x80)
        {
            YELLOWLED_FLASH();
            YellowStutue &= 0x7F;
        }
        else if (LedYELLOWTimer == 1)
            YELLOWLED_FLASH();
    }
    break;
    default:
        break;
    }

    switch (RedStutue & 0x0f)
    {
    case LEDOFFFLAG:
        REDLED_OFF();
        break;
    case LEDONFLAG:
        LED_RED = LED_ON;
        break;
    case LEDFLASHASECONDFLAG:
    {
        if (RedStutue & 0x80)
        {
            REDLED_FLASH_SECOND();
            RedStutue &= 0x7F;
        }
        if (LedREDTimer == 1)
            RedStutue = LEDOFFFLAG;
    }
    break;
    case LEDFLASHFLAG:
    {
        if (RedStutue & 0x80)
        {
            REDLED_FLASH();
            RedStutue &= 0x7F;
        }
        else if (LedREDTimer == 1)
            REDLED_FLASH();
    }
    break;
    default:
        break;
    }
}
/**
****************************************************************************
* @Function	: void CG2214M6_GPIO_Init(void)
* @file		: Initial.c
* @Author	: Xiaowine
* @date		: 2017/4/11
* @version	: V1.0
* @brief	:
**/
void CG2214M6_GPIO_Init(void)
{
    CG2214M6_VC1_DDR = Output; /* 设置数据方向寄存器 1为输出，0为输入--查看STM8寄存器RM0031.pdf 10.9 */
    CG2214M6_VC1_CR1 = 1;      /* 设置推挽输出--查看STM8寄存器RM0031.pdf 10.9*/
    CG2214M6_VC1_CR2 = 1;      /* 设置输出频率 1为10M，0为2M--查看STM8寄存器.pdf P89 */

    CG2214M6_VC2_DDR = Output; /* 设置数据方向寄存器 1为输出，0为输入--查看STM8寄存器.RM0031.pdf 10.9 */
    CG2214M6_VC2_CR1 = 1;      /* 设置推挽输出--查看STM8寄存器RM0031.pdf 10.9*/
    CG2214M6_VC2_CR2 = 1;      /* 设置输出频率 1为10M，0为2M--查看STM8寄存器.pdf P89 */
}

/**
****************************************************************************
* @Function : void ADF7030_GPIO_INIT(void)
* @File     : Initial.c
* @Program  :
* @Created  : 2017/4/12 by Xiaowine
* @Brief    : 芯片外围控制IO初始化
* @Version  : V1.0
**/
void ADF7030_GPIO_INIT(void)
{
    ADF7030_REST_DDR = Output; //* 设置数据方向寄存器 1为输出，0为输入--查看STM8寄存器RM0031.pdf 10.9 */
    ADF7030_REST_CR1 = 1;      //* 设置推挽输出--查看STM8寄存器RM0031.pdf 10.9*/
    ADF7030_REST_CR2 = 1;      //* 设置输出频率 1为10M，0为2M--查看STM8寄存器.pdf P89 */

    ADF7030_GPIO3_DDR = Input; //输入
    ADF7030_GPIO3_CR1 = 1;     //1: Input with pull-up 0: Floating input
    ADF7030_GPIO3_CR2 = 0;     //禁止中断

    // ADF7030_GPIO4_DDR = Input; //输入
    // ADF7030_GPIO4_CR1 = 0;     //1: Input with pull-up 0: Floating input
    // ADF7030_GPIO4_CR2 = 1;     //使能中断
    // EXTI_CR2 &= (~MASK_EXTI_CR2_P4IS);
    // EXTI_CR2 |= 0x02;

    // ADF7030_GPIO5_DDR = Input; //输入
    // ADF7030_GPIO5_CR1 = 1;     //1: Input with pull-up 0: Floating input
    // ADF7030_GPIO5_CR2 = 0;     //禁止中断
}
/**
****************************************************************************
* @Function	: void KEY_GPIO_Init(void)
* @file		: Initial.c
* @Author	: Xiaowine
* @date		: 2017/4/10
* @version	: V1.0
* @brief
**/
void KEY_GPIO_Init(void)
{
    // KEY_SW2_DDR = Input; //输入
    // KEY_SW2_CR1 = 1;     //1: Input with pull-up 0: Floating input
    // KEY_SW2_CR2 = 0;     //禁止中断

    // KEY_SW3_DDR = Input; //输入
    // KEY_SW3_CR1 = 1;     //1: Input with pull-up 0: Floating input
    // KEY_SW3_CR2 = 0;     //禁止中断

    // KEY_SW4_DDR = Input; //输入
    // KEY_SW4_CR1 = 1;     //1: Input with pull-up 0: Floating input
    // KEY_SW4_CR2 = 0;     //禁止中断

    Receiver_Login_direc = Input;          // Input   受信机登录键   低电平有效
    Receiver_Login_CR1 = Pull_up;          //1: Input with pull-up 0: Floating input
    Receiver_Login_CR2 = InterruptDisable; //禁止中断
    //   Receiver_test_direc = Input;
    // Receiver_test_CR1 = 1;

    WORK_TEST_DDR = Input;            // 输入     test脚
    WORK_TEST_CR1 = Pull_up;          //1: Input with pull-up 0: Floating input
    WORK_TEST_CR2 = InterruptDisable; //禁止中断
}
/**
****************************************************************************
* @Function : u8 KEY_SCAN(u8 mode)
* @File     : Initial.c
* @Program  : mode 1 连续按键 0 单次按键
* @Created  : 2017/4/13 by Xiaowine
* @Brief    : 按键扫描
* @Version  : V1.0
**/
u8 KEY_SCAN(u8 mode)
{
    //    static u8 key_up = 1;
    //    if (mode)
    //        key_up = 1;
    //    if ((key_up) && ((KEY_SW2 == 0) || (KEY_SW3 == 0) || (KEY_SW4 == 0)))
    //    {
    //        DELAY_30U();
    //        key_up = 0;
    //        if (KEY_SW2 == 0)
    //            return KEY_SW2_Down;
    //        else if (KEY_SW3 == 0)
    //            return KEY_SW3_Down;
    //        else if (KEY_SW4 == 0)
    //            return KEY_SW4_Down;
    //    }
    //    else if ((KEY_SW2 == 1) && (KEY_SW3 == 1) && (KEY_SW4 == 1))
    //        key_up = 1;
    //    return KEY_Empty;
    return 0;
}
/**
 ****************************************************************************
 * @Function : void RF_BRE_Check(void)
 * @File     : Initial.c
 * @Program  :
 * @Created  : 2017/5/5 by Xiaowine
 * @Brief    :
 * @Version  : V1.0
**/
void RF_BRE_Check(void)
{
    static u8 ErrState = 0;
    u8 j;
    ClearWDT(); // Service the WDT

    if (ErrStateTimeer == 1)
    {
        ErrStateTimeer = 0;
        ErrState = 1;
    }
    if (ErrState == 1)
    {
        if ((RedStutue & 0x7f) != LEDFLASHFLAG)
            RedStutue = LEDFLASHFLAG | 0x80;
    }
    else
        RedStutue = LEDONFLAG;

    if (ADF7030_GPIO3 == 1)
    {
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状态
        DELAY_30U();
        ADF7030_WRITE_REGISTER_NOPOINTER_LONGADDR_MSB(ADDR_IRQ0STATUS, 0xffffffff);
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状态
        DELAY_30U();
        ADF7030_WRITE_REGISTER_NOPOINTER_LONGADDR_MSB(ADDR_IRQ1STATUS, 0xffffffff);
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状态
        DELAY_30U();
        while (ADF7030_GPIO3 == 1)
            ;
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状态
        DELAY_30U();
        ADF7030_CHANGE_STATE(STATE_PHY_ON);
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状态
        ADF7030_RECEIVING_FROM_POWEROFF();
    }

    if (X_COUNT >= 1000)
    {
        if (X_ERR >= 50)
        {
            YellowStutue = LEDOFFFLAG;
        }
        else
        {
            YellowStutue = LEDONFLAG;
            ErrState = 0;
            ErrStateTimeer = 1200;
        }
        for (j = 0; j < 4; j++)
            //lcd    display_map_xy(70 + j * 6, 45, 5, 8, char_Small + (CacheData[3 - j] - ' ') * 5);
            //        display_map_58_6(70,45,4,CacheData);
            X_ERR = 0;
        X_COUNT = 0;
    }
}
void RF_test_mode(void)
{
    UINT8 uart_data, Boot_i;
    //    Receiver_LED_OUT=1;
    //    for(time_3sec=0;time_3sec<9000;time_3sec++){
    //      Delayus(250);   //80us
    //      ClearWDT(); // Service the WDT
    //    }
    //    Receiver_LED_OUT=0;

    Receiver_LED_OUT = 1;
    for (Boot_i = 0; Boot_i < 2; Boot_i++)
    {
        for (time_3sec = 0; time_3sec < 6000; time_3sec++)
        {
            Delayus(250); //80us
            ClearWDT();   // Service the WDT
           // Send_char(0x05);
        }
        Receiver_LED_OUT = !Receiver_LED_OUT;
    }
    Receiver_LED_OUT = 0;

    while (Receiver_test == 0)
    {
        ClearWDT(); // Service the WDT
                    //if(HA_ERR_signal==0){      //test ADF7021 TX
        if (HA_ERR_signal == 0)
        {
            if (HA_L_signal == 0)
                Tx_Rx_mode = 0;
            else
                Tx_Rx_mode = 1;
        }
        else
        {
            if (HA_L_signal == 0)
                Tx_Rx_mode = 2;
            else
                Tx_Rx_mode = 3;
        }
        if ((Tx_Rx_mode == 0) || (Tx_Rx_mode == 1))
        {
            FG_test_rx = 0;
            Receiver_LED_RX = 0;
            FG_test_tx_off = 0;
            //if(HA_L_signal==0){    //发载波，无调制信号
            if (Tx_Rx_mode == 0)
            {
                Receiver_LED_OUT = 1;
                FG_test_mode = 0;
                FG_test_tx_1010 = 0;
                if (FG_test_tx_on == 0)
                {
                    FG_test_tx_on = 1;
                    //7021_DATA_ ADF7021_DATA_direc = Input;
                    //ttset dd_set_TX_mode_carrier();
                }
            }
            else
            { //发载波，有调制信号
                if (TIMER1s == 0)
                {
                    TIMER1s = 500;
                    Receiver_LED_OUT = !Receiver_LED_OUT;
                }
                FG_test_mode = 1;
                FG_test_tx_on = 0;
                if (FG_test_tx_1010 == 0)
                {
                    FG_test_tx_1010 = 1;
                    //7021_DATA_ ADF7021_DATA_direc = Output;
                    //ttset dd_set_TX_mode_1010pattern();
                }
            }
        }
        //else  {           //test ADF7021 RX
        if ((Tx_Rx_mode == 2) || (Tx_Rx_mode == 3))
        {
            FG_test_rx = 1;
            Receiver_LED_OUT = 0;
            FG_test_mode = 0;
            FG_test_tx_on = 0;
            FG_test_tx_1010 = 0;
            if (FG_test_tx_off == 0)
            {
                FG_test_tx_off = 1;
                //ttset dd_set_RX_mode_test();
                //7021_DATA_ ADF7021_DATA_direc = Input;
            }
            //if(HA_L_signal==0){
            if (Tx_Rx_mode == 2)
                if (TIMER1s == 0)
                {
                    TIMER1s = 500;
                    Receiver_LED_RX = !Receiver_LED_RX;
                }
            if (Tx_Rx_mode == 3)
            {
                if (X_COUNT >= 1200)
                {
                    X_COUNT = 0;
                    if (X_ERR >= 60)
                        Receiver_LED_RX = 0;
                    else
                        Receiver_LED_RX = 1;
                    uart_data = (X_ERR / 1000) + 48; //48;//（X_ERR/1000) + 48;
                    Send_char(uart_data);
                    X_ERR = X_ERR % 1000;
                    uart_data = (X_ERR / 100) + 48; //X_ERR/256;
                    Send_char(uart_data);
                    X_ERR = X_ERR % 100;
                    uart_data = (X_ERR / 10) + 48;
                    Send_char(uart_data);
                    X_ERR = X_ERR % 10;
                    uart_data = X_ERR + 48;
                    Send_char(uart_data);
                    uart_data = 13; //|字符
                    Send_char(uart_data);
                    X_ERR = 0;
                }
            }
        }
        PC_PRG(); // PC控制
                  //	if((ADF7021_DATA_CLK==1)&&(FG_test_mode==1)&&(FG_test1==0)){
                  //           ADF7021_DATA_tx=!ADF7021_DATA_tx;
                  //           FG_test1=1;
                  //        }
                  //       if(ADF7021_DATA_CLK==0)FG_test1=0;
    }
  //  UART1_end();
    FG_test_rx = 0;
    TIMER1s = 0;
    Receiver_LED_TX = 0;
    Receiver_LED_RX = 0;
    FG_Receiver_LED_RX = 0;
    Receiver_LED_OUT = 0;

    FLAG_APP_RX = 1;
    //ttset dd_set_RX_mode();
    TIME_Fine_Calibration = 9000;
    TIME_EMC = 10;
}
