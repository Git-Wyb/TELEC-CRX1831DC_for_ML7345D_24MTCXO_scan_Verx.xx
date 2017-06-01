/***********************************************************************/
/*  FILE        :Uart.c                                                */
/*  DATE        :Mar, 2014                                             */
/*  Programmer	:xiang 'R                                              */
/*  CPU TYPE    :STM8L151G6     Crystal: 16M HSI                       */
/*  DESCRIPTION :                                                      */
/*  Mark        :ver 1.0                                               */
/***********************************************************************/
#include <iostm8l151g4.h> // CPU型号
#include "Pin_define.h"   // 管脚定义
#include "initial.h"	  // 初始化  预定义
#include "ram.h"		  // RAM定义
#include "eeprom.h"		  // eeprom
#include "uart.h"
#define TXD1_enable (USART1_CR2 = 0x08) // 允许发送
#define RXD1_enable (USART1_CR2 = 0x24) // 允许接收及其中断

u8 u1busyCache = 0;

UINT8 UartStatus = 0;
UINT8 UartLen = 0;
UINT8 UartCount = 0;
UINT8 UART_DATA_buffer[9] = {0};
__Databits_t Databits_t;
__U1Statues U1Statues;
UINT8 ACKBack[3] = {0x02, 0x03, 0x00};
unsigned int U1AckTimer = 0;

//********************************************
void UART1_INIT(void)
{
	SYSCFG_RMPCR1_USART1TR_REMAP = 0;
	USART1_CR1_bit.M = 1;
	USART1_CR1_bit.PCEN = 1;
	USART1_CR1_bit.PS = 1;
	USART1_CR2_bit.TIEN = 0;
	USART1_CR2_bit.TCIEN = 0;
	USART1_CR2_bit.RIEN = 1;
	USART1_CR2_bit.ILIEN = 0;
	USART1_CR2_bit.TEN = 1;
	USART1_CR2_bit.REN = 1;

//	USART1_CR3 = 0; // 1个停止位
//	USART1_CR4 = 0;
//	USART1_CR5 = 0x00;  //0x08;						// 半双工模式
	USART1_BRR2 = 0x03; // 设置波特率9600
	USART1_BRR1 = 0x68; // 3.6864M/9600 = 0x180
						//16.00M/9600 = 0x683
						//USART1_CR2 = 0x08;	// 允许发送
	//USART1_CR2 = 0x24;
	Send_char(0xa5);
}
void UART1_end(void)
{ //
	SYSCFG_RMPCR1_USART1TR_REMAP = 0;

	USART1_CR1 = 0; // 1个起始位,8个数据位
	USART1_CR3 = 0; // 1个停止位
	USART1_CR4 = 0;
	USART1_CR5 = 0x00;  // 半双工模式
	USART1_BRR2 = 0x00; // 设置波特率9600
	USART1_BRR1 = 0x00; // 3.6864M/9600 = 0x180
						//16.00M/9600 = 0x683
	USART1_CR2 = 0x00;  //禁止串口
}
//--------------------------------------------
void UART1_RX_RXNE(void)
{ // RXD中断服务程序
	unsigned char dat;
	dat = USART1_DR; // 接收数据
					 //Send_char(dat);
	ReceiveFrame(dat);
	// if (dat == '(')
	// 	SIO_cnt = 0;
	// SIO_buff[SIO_cnt] = dat;
	// SIO_cnt = (SIO_cnt + 1) & 0x1F;
	// if (dat == ')')
	// {
	// 	for (dat = 0; dat < SIO_cnt; dat++)
	// 	{
	// 		SIO_DATA[dat] = SIO_buff[dat];
	// 	}
	// 	BIT_SIO = 1; // 标志
	// 				 //SIO_TOT = 20;
	// }
}

//--------------------------------------------
void Send_char(unsigned char ch)
{				 // 发送字符
	TXD1_enable; // 允许发送
	while (!USART1_SR_TXE)
		;
	USART1_DR = ch; // 发送
	while (!USART1_SR_TC)
		;		 // 等待完成发送
	RXD1_enable; // 允许接收及其中断
}
//--------------------------------------------
void Send_String(unsigned char *string)
{ // 发送字符串
	unsigned char i = 0;
	TXD1_enable; // 允许发送
	while (string[i])
	{
		while (!USART1_SR_TXE)
			;				   // 检查发送OK
		USART1_DR = string[i]; // 发送
		i++;
	}
	while (!USART1_SR_TC)
		;		 // 等待完成发送
	RXD1_enable; // 允许接收及其中断
				 //	BIT_SIO = 0;							// 标志
}
void Send_Data(unsigned char *P_data, unsigned int length)
{ // 发送字符串
	unsigned char i = 0;
	TXD1_enable; // 允许发送
	for (i = 0; i < length; i++)
	{
		while (!USART1_SR_TXE)
			;					   // 检查发送OK
		USART1_DR = *(P_data + i); // 发送
	}
	while (!USART1_SR_TC)
		;		 // 等待完成发送
	RXD1_enable; // 允许接收及其中断
				 //	BIT_SIO = 0;							// 标志
}

/***********************************************************************/
unsigned char asc_hex(unsigned char asc) // HEX
{
	unsigned char i;
	if (asc < 0x3A)
		i = asc & 0x0F;
	else
		i = asc - 0x37;
	return i;
}

unsigned char hex_asc(unsigned char hex)
{
	unsigned char i;
	hex = hex & 0x0F;
	if (hex < 0x0A)
		i = hex | 0x30;
	else
		i = hex + 0x37;
	return i;
}

unsigned char asc_hex_2(unsigned char asc1, unsigned char asc0)
{
	unsigned char i;
	i = (asc_hex(asc1) << 4) + (asc_hex(asc0) & 0x0F);
	return i;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void PC_PRG(void) // 串口命令
{
	unsigned int i, j;
	unsigned char d3, d2, d1, d0;

	if (BIT_SIO)
	{
		BIT_SIO = 0;
		//SIO_TOT = 20;
		switch (SIO_DATA[1])
		{
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//%                 写操作               %
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		case 'W':
			//==================================== ADF7012
			if (SIO_DATA[2] == 'I') // (WIxd0d1d2d3)
			{
				i = asc_hex_2(0x30, SIO_buff[3]);
				d0 = asc_hex_2(SIO_buff[4], SIO_buff[5]);
				d1 = asc_hex_2(SIO_buff[6], SIO_buff[7]);
				d2 = asc_hex_2(SIO_buff[8], SIO_buff[9]);
				d3 = asc_hex_2(SIO_buff[10], SIO_buff[11]);

				//write Rx
				ROM_adf7030_value[i].byte[0] = d0;
				ROM_adf7030_value[i].byte[1] = d1;
				ROM_adf7030_value[i].byte[2] = d2;
				ROM_adf7030_value[i].byte[3] = d3;
				//ttset dd_write_7021_reg(&ROM_adf7030_value[i].byte[0]);
				Delayus(122); //delay 40us

				//-------------------------------- 保存
				if (i == 1)
				{
					j = 0x380 + i * 4;
					UnlockFlash(UNLOCK_EEPROM_TYPE);
					WriteByteToFLASH(addr_eeprom_sys + j, d0);
					WriteByteToFLASH(addr_eeprom_sys + j + 1, d1);
					WriteByteToFLASH(addr_eeprom_sys + j + 2, d2);
					WriteByteToFLASH(addr_eeprom_sys + j + 3, d3);
					LockFlash(UNLOCK_EEPROM_TYPE);

					ClearWDT(); // Service the WDT
				}
				//-------------------------------返回  (WHx)
				d1 = '(';
				d2 = 'W';
				Send_char(d1);
				Send_char(d2);
				d1 = 'I';
				d2 = ')';
				Send_char(d1);
				Send_char(d2);
				Send_char(SIO_buff[3]);
			}

			//==================================== ADF7012 TX/RX调试
			if (SIO_DATA[2] == 'J') // (WJx)
			{
				Tx_Rx_mode = asc_hex_2(0x30, SIO_buff[3]);
				//-------------------------------返回  (WHx)
				d1 = '(';
				d2 = 'W';
				Send_char(d1);
				Send_char(d2);
				d1 = 'J';
				d2 = ')';
				Send_char(d1);
				Send_char(d2);
				Send_char(SIO_buff[3]);
			}
			break;
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//%                 读操作               %
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		case 'R':
			//==================================== ADF7012   //(RIx)
			if (SIO_DATA[2] == 'I') // (RI)d0d1d2d3
			{
				i = asc_hex_2(0x30, SIO_buff[3]);
				d1 = '(';
				d2 = 'R';
				Send_char(d1);
				Send_char(d2);
				d1 = 'I';
				d2 = ')';
				Send_char(d1);
				Send_char(d2);
				for (j = 0; j < 4; j++)
				{
					d1 = hex_asc(ROM_adf7030_value[i].byte[j] / 16);
					d2 = hex_asc(ROM_adf7030_value[i].byte[j] % 16);
					Send_char(d1);
					Send_char(d2);
				}
			}
			break;
		default:
			break;
		}
	}
}
void ReceiveFrame(UINT8 Cache)
{
	switch (UartStatus)
	{
	case FrameHeadSataus:
	{
		UART_DATA_buffer[0] = UART_DATA_buffer[1];
		UART_DATA_buffer[1] = UART_DATA_buffer[2];
		UART_DATA_buffer[2] = Cache;
		if ((UART_DATA_buffer[0] == FrameHead) &&
			(UART_DATA_buffer[2] == FrameSingnalID))
		{
			U1Statues = ReceivingStatues;
			UartStatus++;
			UartLen = UART_DATA_buffer[1];
		}
	}
	break;
	case DataStatus:
	{
		UART_DATA_buffer[UartCount + 3] = Cache;
		UartCount++;
		if (UartCount >= (UartLen - 3))
			UartStatus++;
	}
	break;
	default:
		UartStatus = 0;
		U1Statues = IdelStatues;
		break;
	}
	if (UartStatus == 0x02 /*FrameEndStatus*/) //接收完一帧处理数据
	{
		//add Opration function
		OprationFrame();
		UartStatus = 0;
		UartCount = 0;
		//        Receiver_LED_OUT_INV = !Receiver_LED_OUT_INV;
		U1Statues = ReceiveDoneStatues;
		U1AckTimer = U1AckDelayTime;
		U1Busy_OUT = 1;
	}
}

void OprationFrame(void)
{
	unsigned char i;
	for (i = 0; i < 4; i++)
		Databits_t.Data[i] = UART_DATA_buffer[3 + i];
	if (Databits_t.ID_No == 0x92)
	{
		ACKBack[2] = 0;
		switch (Databits_t.Mode)
		{
		case 0:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		default:
			ACKBack[2] = 1;
			return;
			break;
		}
		switch (Databits_t.Statues)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		default:
			ACKBack[2] = 1;
			return;
			break;
		}
		switch (Databits_t.Abnormal)
		{
		case 0x00:
			break;
		case 0x04:
			break;
		case 0x06:
			break;
		case 0x45:
			break;
		case 0x46:
			break;
		case 0x47:
			break;
		case 0x48:
			break;
		default:
			ACKBack[2] = 1;
			return;
			break;
		}
	}
	else
	{
		ACKBack[2] = 1;
		return;
	}
}
void TranmissionACK(void)
{

	if ((U1Statues == ReceiveDoneStatues) && (U1AckTimer == 0))
	{
		U1Busy_OUT = 1;
		U1Statues = ACKingStatues;
		Send_Data(ACKBack, 3);
		U1Statues = IdelStatues;
		U1Busy_OUT = 1;
	}
}
