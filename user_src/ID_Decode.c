/***********************************************************************/
/*  FILE        :ID_Decode.c                                           */
/*  DATE        :Mar, 2014                                             */
/*  Programmer	:xiang 'R                                              */
/*  CPU TYPE    :STM8L151G6     Crystal: 16M HSI                       */
/*  DESCRIPTION :                                                      */
/*  Mark        :ver 1.0                                               */
/***********************************************************************/
#include <iostm8l151g4.h> // CPU型号
#include "Pin_define.h"   // 管脚定义
#include "initial.h"      // 初始�? 预定�?
#include "ram.h"          // RAM定义
#include "ID_Decode.h"
#include "eeprom.h" // eeprom
#include "uart.h"   // uart
#include "ADF7030_1.h"
//void EXIT_init(void)
//{
//    EXTI_CR1 = 0x20;          //PORT B2  的中断触发位
//    ADF7021_DATA_CLK_CR2 = 1; //使能该I/O口中�? PA1
//    //   EXTI_CR2=0X00;
//    //   PIN_PD7_CR2=1;
//}
/**
 ****************************************************************************
 * @Function : void DataReceive(void)
 * @File     : ID_Decode.c
 * @Program  :
 * @Created  : 2017/5/5 by Xiaowine
 * @Brief    : TEST BER
 * @Version  : V1.0
**/
void DataReceive(void)
{
    static u8 StateCache = 0;
    static u8 Cache = 0;
    static u8 X_HISbyte = 0;
    switch (StateCache)
    {
    case 0:
    {
        Cache <<= 1;
        if (ADF7030DATA)
            Cache++;
        if (Cache == 0x55)
        {
            StateCache = 1;
            X_HISbyte = 0;
            Cache = 0;
        }
    }
    break;
    case 1:
    {
        if (ADF7030DATA != X_HISbyte)
            X_ERR++;
        X_COUNT++;
        X_HISbyte ^= 1;
        if (X_COUNT >= 1000)
            StateCache = 2;
    }
    break;
    case 2:
        if (X_COUNT == 0)
            StateCache = 0;
        break;
    default:
        break;
    }
    EXTI_SR1_P4F = 1;
}
/*
void ID_Decode_function(void)
{
    UINT16 DATA_Packet_Syn_bak = 0;
    if (FLAG_APP_RX == 1)
    {
        TIME_EMC = 10;
        switch (rxphase)
        {
        case 0:
            DATA_Packet_Syn = DATA_Packet_Syn << 1;
            if (ADF7021_DATA_rx)
                DATA_Packet_Syn += 1;
            if (TIMER18ms == 0)
            {
                DATA_Packet_Syn_bak = DATA_Packet_Syn & 0x0000FFFF;
                if ((DATA_Packet_Syn_bak == 0x5555) || (DATA_Packet_Syn_bak == 0xAAAA))
                    ;
                else
                    FLAG_Receiver_Scanning = 1;
            }
            //if(DATA_Packet_Syn==0x55555555){rxphase=1;TIMER18ms=65;DATA_Packet_Syn=0;DATA_Packet_Head=0;}
            if ((DATA_Packet_Syn & 0xFFFFFFFF) == 0x55555555)
            {
                rxphase = 1;
                TIMER18ms = 2000;
                DATA_Packet_Syn = 0;
                DATA_Packet_Head = 0;
                //Receiver_LED_RX=1;
                FG_Receiver_LED_RX = 1;
                TIMER300ms = 500;
            }
            break;
        case 1:
            DATA_Packet_Head = DATA_Packet_Head << 1;
            if (ADF7021_DATA_rx)
                DATA_Packet_Head += 1;
            //DATA_Packet_Head=DATA_Packet_Head&0x0000FFFF;
            if (TIMER18ms == 0)
                rxphase = 0;
            if (DATA_Packet_Head == 0x5515)
            {
                rxphase = 2;
                DATA_Packet_Syn = 0;
                DATA_Packet_Head = 0;
                DATA_Packet_Code_i = 0;
            }
            break;
        case 2:
            DATA_Packet_Code_g = DATA_Packet_Code_i / 32;
            DATA_Packet_Code[DATA_Packet_Code_g] = DATA_Packet_Code[DATA_Packet_Code_g] << 1;
            if (ADF7021_DATA_rx)
                DATA_Packet_Code[DATA_Packet_Code_g] += 1;
            DATA_Packet_Code_i++;
            if (DATA_Packet_Code_i == 96)
            {
                if ((DATA_Packet_Code[1] & 0x0000FFFF) == 0x5556)
                    ;
                else
                    rxphase = 3;
            }
            else if (DATA_Packet_Code_i >= 192)
                rxphase = 3;
            break;
        case 3:
            FLAG_Receiver_IDCheck = 1;
            //                if((Freq_Scanning_CH==1)||(Freq_Scanning_CH==3)||(Freq_Scanning_CH==5))Freq_Scanning_CH_bak=0;   //暂时记录下收到信号的频率信道,0代表426M
            //                else Freq_Scanning_CH_bak=1;                                                                                 //                       1代表429M
            rxphase = 0;
            DATA_Packet_Syn = 0;
            TIMER18ms = 0; //0ms，接受可靠稳�?
            break;
        default:
            break;
        }
    }
    else if (FG_test_rx == 1)
    {
        X_COUNT++;
        if ((ADF7021_DATA_rx == X_HIS) && (X_COUNT != 1))
            X_ERR++;
        X_HIS = ADF7021_DATA_rx;
    }
    EXTI_SR1_bit.P2F = 1;
}
*/
void ID_Decode_IDCheck(void)
{

    if (FLAG_Receiver_IDCheck)
    {
        FLAG_Receiver_IDCheck = 0;
        Signal_DATA_Decode(0);
        if (FLAG_Signal_DATA_OK == 1)
        {
            eeprom_IDcheck();
            if ((FLAG_ID_Erase_Login == 1) || (FLAG_ID_Login == 1))
            {
                if ((FLAG_ID_Login_OK == 0) && (DATA_Packet_Contro_buf != 0x40) && (DATA_Packet_ID != 0)) //2015.4.1修正 在登录模式下 不允许自动送信登录，只允许手动送信登录
                {
                    if ((FLAG_ID_Erase_Login == 1) || ((FLAG_ID_Login == 1)&&(DATA_Packet_ID != 0xFFFFFE)))
                    	{
	                    FLAG_ID_Login_OK = 1;
	                    ID_Receiver_Login = DATA_Packet_ID;
                    	}
                }
            }
            else if ((FLAG_IDCheck_OK == 1) || (DATA_Packet_ID == 0xFFFFFE))
            {
                FLAG_IDCheck_OK = 0;
                if (DATA_Packet_ID == 0xFFFFFE)
                    DATA_Packet_Control = DATA_Packet_Contro_buf; //2015.3.24修正 Control缓存�?ID判断是否学习过后才能使用
                                                                  //                 if (Freq_Scanning_CH_bak == 0)
                                                                  //                 {
                                                                  //                     Freq_Scanning_CH_save = 1;
                                                                  //                     Freq_Scanning_CH_save_HA = 0;
                                                                  //                 } //当前收到426M控制   但保存记录下收到信号的频率信�?0代表426M
                                                                  //                 else
                                                                  //                     Freq_Scanning_CH_save_HA = 1; //                       1代表429M
                                                                  //                 DATA_Packet_Control_0 = DATA_Packet_Control;
                                                                  // #if defined(__Product_PIC32MX2_Receiver__)
                                                                  //                 if (DATA_Packet_Control == 0x08)
                                                                  //                     DATA_Packet_Control_err = 0x08;
                                                                  //                 if (DATA_Packet_Control == 0x02)
                                                                  //                 {
                                                                  //                     DATA_Packet_Control_err = 0x02;
                                                                  //                     FLAG_HA_ERR_bit = 0;
                                                                  //                 }
                                                                  // #endif
                                                                  //                 if (((DATA_Packet_Code[1] & 0x0000FFFF) == 0x5556) && (Freq_Scanning_CH_bak == 0))
                                                                  //                 {
                if ((SPI_Receive_DataForC[1] & 0x0000FFFF) == 0x5556)
                {
                    PAYLOAD_SIZE = RX_PayLoadSizeLogin;
                    Signal_DATA_Decode(1);
                    if (FLAG_Signal_DATA_OK == 1)
                    {
                        eeprom_IDcheck();
                        if (DATA_Packet_Contro_buf == 0xFF)
                        { //2015.4.2修正  送信器旁边的登录�?追加登录不行
                            if (FLAG_IDCheck_OK == 1)
                                FLAG_IDCheck_OK = 0;
                            else if (ID_DATA_PCS < 256)
                            {
                                BEEP_and_LED();
                                ID_Receiver_Login = DATA_Packet_ID;
                                ID_EEPROM_write();
                            } //end else
                        }
                        else if (DATA_Packet_Control == 0x00)
                        {
                            if (FLAG_IDCheck_OK == 1)
                            {
                                FLAG_IDCheck_OK = 0;
                                BEEP_and_LED();
                                ID_EEPROM_write_0x00();
                            }
                        }
                    }
                }
                else
                {
                    PAYLOAD_SIZE = RX_PayLoadSizeNOLogin;
                    //#if defined(__Product_PIC32MX2_WIFI__)
                    //                    TIMER1s=500;//1000
                    //#endif
                    //#if defined(__Product_PIC32MX2_Receiver__)
                    //                    if(Freq_Scanning_CH_bak==0){
                    if ((DATA_Packet_Control == 0x40) && (Manual_override_TIMER == 0))
                    {
                        FG_auto_manual_mode = 1;
                        TIME_auto_out = 890; // 900
                        if (FG_First_auto == 0)
                        {
                            FG_First_auto = 1;
                            TIMER1s = 3000; //2500
                        }
                    }
                    else if (DATA_Packet_Control == 0x40)
                        ;
                    else
                    {
                        FG_auto_out = 0;
                        TIME_auto_close = 0;
                        FG_auto_open_time = 0;
                        if (FG_auto_manual_mode == 1)      //Manual_override_TIMER=13500;   //2�?0秒自动无�?
                            Manual_override_TIMER = 24480; //4�?0秒自动无�?
                        if ((DATA_Packet_Control & 0x14) == 0x14)
                        {
                            if (TIMER1s == 0)
                                TIMER1s = 3800 - 30;
                        }
                        else
                            TIMER1s = 1000;
                    }
                    //                    }
                    //                    else TIMER1s=1000;
                    TIMER300ms = 500;
                    //Receiver_LED_RX=1;
                    FG_Receiver_LED_RX = 1;
                    //#endif
                }
            }
        }
    }
}

void Signal_DATA_Decode(UINT8 NUM_Type)
{
    UINT32 data_in;
    UINT16 data_out;
    UINT16 data_NRZ[3];
    UINT8 i, j;
    for (i = 0; i < 3; i++)
    {
        if (NUM_Type == 0)
            data_in = SPI_Receive_DataForC[i];
        else
            data_in = SPI_Receive_DataForC[i + 3];
        data_out = 0;
        data_in = data_in >> 1;
        for (j = 0; j < 16; j++)
        {
            data_out = data_out << 1;
            if (data_in & 0x00000001)
                data_out += 1;
            data_in = data_in >> 2;
        }
        data_NRZ[i] = data_out;
    }
    if (data_NRZ[2] == ((data_NRZ[0] + data_NRZ[1]) & 0xFFFF))
    {
        FLAG_Signal_DATA_OK = 1;
        LCDUpdateIDFlag = 1;
        DATA_Packet_ID = (data_NRZ[1] & 0x00FF) * 65536 + data_NRZ[0];
        if (DATA_Packet_ID == 0)
            FLAG_Signal_DATA_OK = 0;                          //2014.3.21追加  不允许使用ID=0
        DATA_Packet_Contro_buf = (data_NRZ[1] & 0xFF00) >> 8; //2015.3.24修正 Control缓存�?ID判断是否学习过后才能使用
                                                              //          if(Freq_Scanning_CH_bak==1)
                                                              //              Control_bak=DATA_Packet_Control;
    }
    else
        FLAG_Signal_DATA_OK = 0;
}

void eeprom_IDcheck(void)
{
    UINT16 i;
	if(FLAG_testNo91==1)
		{
            FLAG_IDCheck_OK = 1;
            DATA_Packet_Control = DATA_Packet_Contro_buf;		
		}
    for (i = 0; i < ID_DATA_PCS; i++)
    {
        if (ID_Receiver_DATA[i] == DATA_Packet_ID)
        {
            INquiry = i;
            i = ID_DATA_PCS;
            FLAG_IDCheck_OK = 1;
            DATA_Packet_Control = DATA_Packet_Contro_buf;
        } //2015.3.24修正 Control缓存�?ID判断是否学习过后才能使用
        if ((FLAG_ID_Erase_Login == 1) && (FLAG_ID_Erase_Login_PCS == 1))
        {
            i = ID_DATA_PCS;
            FLAG_IDCheck_OK = 0;
            DATA_Packet_Control = DATA_Packet_Contro_buf;
        } //追加多次ID登录
    }
}
/*
   time_beepON、time_beepOFF单位时间为0.4333333ms
*/
void BEEP_Module(UINT16 time_beepON, UINT16 time_beepOFF)
{
	UINT16 i;

	for (i = 0; i < time_beepON; i++)
	{
		//Receiver_Buzzer=!Receiver_Buzzer;   //蜂鸣器频�?.08KHZ
		if (FG_beep_on == 0)
		{
			FG_beep_on = 1;
			FG_beep_off = 0;
			BEEP_CSR2_BEEPEN = 1;
		}
		Delayus(250); //80us
		Delayus(250); //80us
		Delayus(250); //80us
		ClearWDT();   // Service the WDT
	}
	for (i = 0; i < time_beepOFF; i++)
	{
		// Receiver_Buzzer=0;	//蜂鸣器频�?.08KHZ
		if (FG_beep_off == 0)
		{
			FG_beep_off = 1;
			FG_beep_on = 0;
			BEEP_CSR2_BEEPEN = 0;
		}
		//Delayus(240);
		Delayus(250); //80us
		Delayus(250); //80us
		Delayus(250); //80us
		ClearWDT();   // Service the WDT
	}

}

void BEEP_and_LED(void)
{
    Receiver_LED_OUT = 1;
    BEEP_Module(4160,0);
    FG_beep_on = 0;
    BEEP_CSR2_BEEPEN = 0;
    TIME_Receiver_LED_OUT = 185;
}

void Receiver_BEEP(void)
{
    UINT16 j;
    if (FLAG_Receiver_BEEP == 0)
    {
        FLAG_Receiver_BEEP = 1;
        for (j = 0; j < 3; j++)
          BEEP_Module(1600,900);
    }
}

void TEST_beep(void)
{
	if(FLAG_testBEEP==1)
		BEEP_Module(300,1);
	else if(FLAG_testBEEP==2)
		{
		BEEP_Module(300,900);
		BEEP_Module(300,1);
		}
	else if(FLAG_testBEEP==3) BEEP_CSR2_BEEPEN = 1;
	FLAG_testBEEP=0;
}

void ID_Decode_OUT(void)
{
    UINT8 Control_i;
	

    //    if(Freq_Scanning_CH_bak==0) Control_i=DATA_Packet_Control&0xFF;
    //    else Control_i=DATA_Packet_Control&0x0E;
    //    if(HA_Sensor_signal==1)Receiver_LED_TX=0;                      //test 接近信号回路
    //     else Receiver_LED_TX=1;

    Control_i = DATA_Packet_Control & 0xFF;
    if (TIMER1s)
    {
      if((FLAG_testNo91==1)||(FLAG_testNo91==2))
      {
			if((FLAG_testNo91SendUart==0)&&(FLAG_testNo91==1))
				{
				       switch (Control_i)
                       {
				        case 0x08: //open
				            if(FLAG_testNo91_step==0)
				            	{
					            Receiver_LED_OUT = 1;
								ACKBack[2]=0xA1;
								Send_Data(ACKBack, 3);
								FLAG_testNo91SendUart=1;	
				            	}
				            break;   
				        case 0x04: //stop
				            if(FLAG_testNo91_step==1)
				            	{
					            Receiver_LED_OUT = 1;
								ACKBack[2]=0xA2;
								Send_Data(ACKBack, 3);
								FLAG_testNo91SendUart=1;	
				            	}						
				            break; 							
				        case 0x02: //close
				            if(FLAG_testNo91_step==2)
				            	{
					            Receiver_LED_OUT = 1;
								ACKBack[2]=0xA4;
								Send_Data(ACKBack, 3);
								FLAG_testNo91SendUart=1;	
				            	}						
				            break;
						default:
                            break;	
                       }
				
				}        	
      }
	  else 
	  {
        switch (Control_i)
        {
        case 0x14: //stop+login
            Receiver_LED_OUT = 1;
            TIMER250ms_STOP = 250;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_STOP = FG_allow_out;
            if (TIMER1s < 3550)
            {
                Receiver_OUT_OPEN = FG_allow_out;
                Receiver_OUT_CLOSE = FG_allow_out;
                Receiver_BEEP();
            }
            break;
        case 0x40: //自动送信
            if ((FG_auto_out == 0) && (Manual_override_TIMER == 0))
            {
                Receiver_LED_OUT = 1;
                TIMER250ms_STOP = 0;
                Receiver_OUT_VENT = FG_NOT_allow_out;
                Receiver_OUT_CLOSE = FG_NOT_allow_out;
                if (TIMER1s > 2000)
                {
                    Receiver_OUT_STOP = FG_allow_out;
                    Receiver_OUT_OPEN = FG_NOT_allow_out;
                } //1830
                else if (TIMER1s > 1000)
                {
                    Receiver_OUT_STOP = FG_NOT_allow_out;
                    Receiver_OUT_OPEN = FG_NOT_allow_out;
                } //810
                else
                {
                    FG_auto_open_time = 1;
                    Receiver_OUT_STOP = FG_NOT_allow_out;
                    Receiver_OUT_OPEN = FG_allow_out;
                }
            }
            break;
        case 0x01: //VENT
            Receiver_LED_OUT = 1;
            if (Receiver_vent == 0)
            { //受信机换气联动OFF
                Receiver_OUT_STOP = FG_NOT_allow_out;
                Receiver_OUT_VENT = FG_NOT_allow_out;
                Receiver_OUT_OPEN = FG_allow_out;
                Receiver_OUT_CLOSE = FG_allow_out;
            }
            else
            { //受信机换气联动ON
                Receiver_OUT_STOP = FG_NOT_allow_out;
                Receiver_OUT_OPEN = FG_NOT_allow_out;
                Receiver_OUT_CLOSE = FG_NOT_allow_out;
                Receiver_OUT_VENT = FG_allow_out;
            }
            break;
        case 0x02: //close
            Receiver_LED_OUT = 1;
            Receiver_OUT_OPEN = FG_NOT_allow_out;
            Receiver_OUT_STOP = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_CLOSE = FG_allow_out;
            break;
        case 0x04: //stop
            Receiver_LED_OUT = 1;
            Receiver_OUT_OPEN = FG_NOT_allow_out;
            Receiver_OUT_CLOSE = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_STOP = FG_allow_out;
            break;
        case 0x08: //open
            Receiver_LED_OUT = 1;
            Receiver_OUT_STOP = FG_NOT_allow_out;
            Receiver_OUT_CLOSE = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_OPEN = FG_allow_out;
            break;
        case 0x0C: //open+stop
            Receiver_LED_OUT = 1;
            TIMER250ms_STOP = 250;
            Receiver_OUT_CLOSE = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_STOP = FG_allow_out;
            if (FG_OUT_OPEN_CLOSE == 0)
            {
                FG_OUT_OPEN_CLOSE = 1;
                TIME_OUT_OPEN_CLOSE = 25;
            }
            if (TIME_OUT_OPEN_CLOSE == 0)
                Receiver_OUT_OPEN = FG_allow_out;
            break;
        case 0x06: //close+stop
            Receiver_LED_OUT = 1;
            TIMER250ms_STOP = 250;
            Receiver_OUT_OPEN = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_STOP = FG_allow_out;
            if (FG_OUT_OPEN_CLOSE == 0)
            {
                FG_OUT_OPEN_CLOSE = 1;
                TIME_OUT_OPEN_CLOSE = 25;
            }
            if (TIME_OUT_OPEN_CLOSE == 0)
                Receiver_OUT_CLOSE = FG_allow_out;
            break;
        case 0x0A: //close+OPEN
            Receiver_LED_OUT = 1;
            Receiver_OUT_STOP = FG_NOT_allow_out;
            Receiver_OUT_VENT = FG_NOT_allow_out;
            Receiver_OUT_OPEN = FG_allow_out;
            Receiver_OUT_CLOSE = FG_allow_out;
            break;
        case 0x09: //vent+OPEN
            Receiver_LED_OUT = 1;
            Receiver_OUT_STOP = FG_NOT_allow_out;
            Receiver_OUT_CLOSE = FG_NOT_allow_out;
            Receiver_OUT_OPEN = FG_allow_out;
            Receiver_OUT_VENT = FG_allow_out;
            break;
        case 0x03: //vent+close
            Receiver_LED_OUT = 1;
            Receiver_OUT_STOP = FG_NOT_allow_out;
            Receiver_OUT_OPEN = FG_NOT_allow_out;
            Receiver_OUT_CLOSE = FG_allow_out;
            Receiver_OUT_VENT = FG_allow_out;
            break;
        default:
            break;
        }
	  	}
    }
    else
    {
        //           if(FLAG_APP_Reply==1){FLAG_APP_Reply=0;ID_data.IDL=DATA_Packet_ID;Control_code=HA_Status;FLAG_HA_START=1;}
        //           if(FLAG_426MHz_Reply==1){FLAG_426MHz_Reply=0;ID_data.IDL=DATA_Packet_ID;Control_code=HA_Status+4;FLAG_HA_START=1;}   //受信器自动发送HA状态码为实际HA�?4
        if ((FG_auto_out == 1) && (TIME_auto_out == 0))
        {
            FG_auto_out = 0;
            TIME_auto_close = 270;
            Receiver_LED_OUT = 1;
        } //300
        if (TIME_auto_close)
        {
            if (TIME_auto_close > 180)
            {
                Receiver_OUT_STOP = FG_allow_out;
                Receiver_OUT_CLOSE = FG_NOT_allow_out;
            } //200
            else if (TIME_auto_close > 90)
            {
                Receiver_OUT_STOP = FG_NOT_allow_out;
                Receiver_OUT_CLOSE = FG_NOT_allow_out;
            } //100
            else
            {
                Receiver_OUT_STOP = FG_NOT_allow_out;
                Receiver_OUT_CLOSE = FG_allow_out;
            }
        }
        else
        {
            FG_auto_manual_mode = 0;
            Receiver_OUT_CLOSE = FG_NOT_allow_out;
        }
        FG_First_auto = 0;
        FLAG_Receiver_BEEP = 0;
        if ((FLAG_ID_Erase_Login == 1) || (FLAG_ID_Login == 1) || (TIME_auto_close))
            ;
        else if (TIME_Receiver_LED_OUT > 0)
            Receiver_LED_OUT = 1;
        else if(TIME_power_led==0)
            Receiver_LED_OUT = 0;
        Receiver_OUT_OPEN = FG_NOT_allow_out;
        Receiver_OUT_VENT = FG_NOT_allow_out;
        if (FG_auto_open_time == 1)
        {
            FG_First_auto = 0;
            FG_auto_out = 1;
            FG_auto_open_time = 0;
        }
        if ((TIMER250ms_STOP == 0) && (TIME_auto_close == 0))
        {
            Receiver_OUT_STOP = FG_NOT_allow_out;
            FG_OUT_OPEN_CLOSE = 0;
        }
		if(FLAG_testNo91_step==2)FLAG_testNo91SendUart=0;
    }
    if (TIMER300ms == 0)
        FG_Receiver_LED_RX = 0; //Receiver_LED_RX=0;
}

void Freq_Scanning(void)
{
    if (TIMER18ms == 0)
    {
        if (Flag_FREQ_Scan == 0)
        {
            if (ADF7030_Read_RESIGER(0x4000380C, 1, 0) != 0)
            {
                Flag_FREQ_Scan = 1;
                TIMER18ms = 80;
                return;
            }
        }
        while (GET_STATUE_BYTE().CMD_READY == 0)
            ;
        DELAY_30U();
        ADF7030_CHANGE_STATE(STATE_PHY_ON);
        ADF7030_Change_Channel();
        GET_STATUE_BYTE();
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状�?
        DELAY_30U();
        ADF7030_CHANGE_STATE(STATE_PHY_ON);
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状�?
        DELAY_30U();
        ADF7030_WRITE_REGISTER_NOPOINTER_LONGADDR_MSB(ADDR_GENERIC_PKT_FRAME_CFG1, GENERIC_PKT_FRAME_CFG1);    //
        ADF7030_WRITE_REGISTER_NOPOINTER_LONGADDR_MSB(ADDR_CHANNEL_FERQUENCY, PROFILE_CH_FREQ_32bit_200002EC); //
        WaitForADF7030_FIXED_DATA();                                                                           //等待芯片空闲/可接受CMD状�?
        DELAY_30U();
        ADF7030_Clear_IRQ();
        WaitForADF7030_FIXED_DATA(); //等待芯片空闲/可接受CMD状�?
        DELAY_30U();
        ADF7030_CHANGE_STATE(STATE_PHY_RX);
        while (GET_STATUE_BYTE().FW_STATUS == 0)
            ;
        DELAY_30U();
        //ADF7030_RECEIVING_FROM_POWEROFF();
        while (GET_STATUE_BYTE().FW_STATUS != 1)
            ;
        while (ADF7030_GPIO3 == 1)
            ;
        TIMER18ms = 15;
        Flag_FREQ_Scan = 0;
    }

    if (((FLAG_Receiver_Scanning == 1) || (TIME_EMC == 0) || (TIME_Fine_Calibration == 0)) && (FLAG_APP_RX == 1))
    {
        FLAG_Receiver_Scanning = 0;
        if (TIME_Fine_Calibration == 0)
        {
            TIME_Fine_Calibration = 900;

            //ttset dd_set_ADF7021_Power_on();
            //ttset dd_set_RX_mode();
        }
        //ttset dd_set_ADF7021_Freq();
    }
}
