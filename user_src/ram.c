#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <stdbool.h> /* For true/false definition                      */
//#include "initial.h"		// ?????  ?????
#include "type_def.h"
#include "ram.h"

Mark_Un mark0_stu = {0};
Mark_Un mark1_stu = {0};

u8 DEF_APP_TX_freq=3;

uFLAG FLAG_test1;
volatile uFLAG FLAG0;
volatile uFLAG FLAG1;
volatile uFLAG FLAG_test;
volatile uFLAG FLAG2;
volatile uFLAG FLAG3;


u16 X_COUNT = 0;
u16 X_ERR = 0; //记录错误的个�?
u16 X_ERRTimer = 0;

u8 SIO_cnt = 0;
u8 SIO_buff[16] = {0};
u8 SIO_DATA[16] = {0};
u8 Tx_Rx_mode = 0;

u8 TIME_10ms = 0;
u16 TIMER1s = 0;
u16 TIMER300ms = 0;
u16 TIMER18ms = 0;
u16 TIMER250ms_STOP = 0;
u16 TIME_auto_out = 0;
u16 TIME_auto_close = 0;
u16 time_3sec = 0;
u32 ID_Receiver_DATA[256] = {0}; //写入EEPROM ID缓存
u32 ID_SCX1801_DATA = 1;
u16 ID_DATA_PCS = 0;
u32 DATA_Packet_ID = 0;
u8 DATA_Packet_Control = 0;
u8 DATA_Packet_Contro_buf = 0; //2015.3.24修正
u32 ID_Receiver_Login = 0;
u8 TIME_EMC = 0; //静电测试
u8 LED_Cache = 0;

u16 INquiry = 0;
u16 TIME_Receiver_Login_restrict = 0;
u8 COUNT_Receiver_Login = 0;
u16 TIME_Receiver_Login = 0;
u16 TIME_Login_EXIT_rest = 0;
u16 TIME_Receiver_Login_led = 0;
u16 TIME_ID_SCX1801_Login = 0;


u8 TIME_OUT_OPEN_CLOSE = 0;
u16 TIME_Receiver_LED_OUT = 0;
u16 TIME_Login_EXIT_Button = 0;
u16 Manual_override_TIMER = 0;
u16 time_Login_exit_256 = 0;
u16 TIME_Fine_Calibration = 0; //窄带下中频滤波器100KHz精校

u8 FLAG_TELEC_mode=0;
u8 FLAG_TELEC_CH=0;
u8 TIME_TELEC_mode=0;
u8 TIME_TELEC_CH=0;
u8 FLAG_TELEC_CH_dec=0;
u8 TIME_TELEC_CH_dec=0;
u8 TELEC_Frequency_CH=2;
u8 FLAG_TELEC_10ms=0;

u8 Count_key_SW3 = 0;
u8 Display_key_SW3 = 0xff;

u16 TIME_TestNo91=0;
u16 TIME_power_led=0;


/*******LCD**************/
uFLAG FlagLCDUpdate = {0xff};
///**********RSSI*********************/
short RAM_RSSI_AVG = 0;
long RAM_RSSI_SUM = 0;
u8 RSSI_Read_Counter = 0;
u8 Flag_RSSI_Read_Timer = 10;

const u8 ADF7030Cfg[] = {
#include "Settings_ADF7030-1.cfg"
};
const u8 ADF7030Cfg_load[] = {
#include "Settings_ADF7030-1_load.cfg"
};
const u8 ADF7030Cfg_4dot8k[] = {
#include "Settings_ADF7030-1_4dot8k.cfg"
};
const u8 *ADF7030Cfg_pointer=ADF7030Cfg;

/**
****************************************************************************
* @Function : u32 CFG_SIZE(void)
* @File     : ram.c
* @Program  :
* @Created  : 2017/4/24 by Xiaowine
* @Brief    :
* @Version  : V1.0
**/
u32 CFG_SIZE(void)
{
	return sizeof(ADF7030Cfg);
}
u8 CONST_TXPACKET_DATA_20000AF0[28] = {
	0X95, 0X55, 0X55, 0X55,
	0X55, 0X55, 0X56, 0X55,
	0X95, 0X55, 0X56, 0X55,
	0X95, 0X55, 0X55, 0X55,
	0X95, 0X55, 0X55, 0X55,
	0X95, 0X55, 0X55, 0X55,
	0X95, 0X55, 0X55, 0X55};

u32 GENERIC_PKT_TEST_MODES0_32bit_20000548 = 0x00000000;
const u8 TEST_MODES0_para[5] = {0, 1, 2, 4, 6};
u32 RADIO_DIG_TX_CFG0_32bit_20000304 = 0xC838287E;
u16 PA_POWER_OUT[2][18] = {
	{0x0382, 0x02a3, 0x02f3, 0x0353, 0x02d4,
	 0x0295, 0x02e5, 0x02b6, 0x0316, 0x0376,
	 0x03c6, 0x0486, 0x0536, 0x05f6, 0x05f6,
	 0x05f6, 0x05f6, 0x05f6},
	{0x0382, 0x02a3, 0x02f3, 0x0353, 0x02d4,
	 0x0295, 0x02e5, 0x02b6, 0x02a7, 0x0298,
	 0x02a9, 0x02aa, 0x031a, 0x037a, 0x040a,
	 0x04aa, 0x057a, 0x069a}};
const u8 CONST_AFC_Configuration_400041F8[4] = {
	0x00, 0x00, 0x00, 0x00 /*20000304*/
};

const u8 CONST_PROFILE_CCA_CFG_20000378[4] = {
	0x43, 0x00, 0xC0, 0x06 /*20000304*/
};
ADF70XX_REG_T ROM_adf7030_value[16];
const ADF70XX_REG_T Default_adf7030_value[16] = {
	0x00000000, 0x031B5011, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};
u32 PROFILE_CH_FREQ_32bit_200002EC = 426075000ul;
const u32 PROFILE_CH1_FREQ_32bit_429HighSpeed=429350000;//429350000;//429225000;
const u32 PROFILE_CH2_FREQ_32bit_429HighSpeed=429550000;//429550000;//429237500;
u8 Channels=1;


Wireless_Body Struct_DATA_Packet_Contro,Struct_DATA_Packet_Contro_buf;
Wireless_Body Uart_Struct_DATA_Packet_Contro,Last_Uart_Struct_DATA_Packet_Contro;
u8 Struct_DATA_Packet_Contro_fno=0;
u16 TIMER_Semi_open;
u16 TIMER_Semi_close;
u8 FLAG__Semi_open_T=0;
u8 FLAG__Semi_close_T=0;
u16 TIME_APP_TX_fromOUT=0;
u8 Time_APP_blank_TX=0;
u16 Time_APP_RXstart=0;

u8 TIME_TX_RSSI_Scan=0;
u8 Flag_TX_ID_load=0;

u8 TIME_ERROR_Read_once_again=0;
unsigned char Send_err_com[7] = {0x02, 0x07, 0x11,0x98,0x09,0x52,0x46};
u8 Time_error_read_gap=0;
u16 Time_error_read_timeout=0;
u8 ERROR_Read_sendTX_count=0;
u8 ERROR_Read_sendTX_packet=0;
u8 Flag_ERROR_Read_once_again=0;


u32 PROFILE_CH_FREQ_32bit_200002EC_TELEC = 426075000ul;


