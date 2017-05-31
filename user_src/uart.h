#ifndef __UART_H__
#define __UART_H__

/***********************************************************************/
/*  FILE        :Uart.H                                                */
/*  DATE        :Mar, 2014                                             */
/*  Programmer	:xiang 'R                                              */
/*  CPU TYPE    :STM8L151G6     Crystal: 16M HSI                       */
/*  DESCRIPTION :                                                      */
/*  Mark        :ver 1.0                                               */
/***********************************************************************/

extern u8 u1busyCache;
#define U1Busy_OUT u1busyCache
#define FrameHeadSataus 0x00 //帧头
#define DataStatus 0x01      //数据位置
#define FrameEndStatus 0x02  //桢结束
#define FrameHead 0x02       //数据开始
#define FrameSingnalID 0x11  //信号ID

extern UINT8 UartStatus;
extern UINT8 UartLen;
extern UINT8 UartCount;
extern UINT8 UART_DATA_buffer[9];
void ReceiveFrame(UINT8 Cache);
void OprationFrame(void);
void TranmissionACK(void);
typedef union {
    unsigned char Data[4];
    struct
    {
        unsigned ID_No : 8;
        unsigned Statues : 4;
        unsigned Mode : 4;
        unsigned Abnormal : 8;
        unsigned Standby : 8;
    };
    struct
    {
        unsigned : 8;
        unsigned SW_Info : 8;
        unsigned AbnormalOut1 : 8;
        unsigned AbnormalOut2 : 8;
    };
} __Databits_t;
typedef enum {
    IdelStatues = 0,
    ReceivingStatues,
    ReceiveDoneStatues,
    ACKingStatues,
    ACKDoneStatues,
} __U1Statues;
extern __U1Statues U1Statues;
extern unsigned int U1AckTimer;
#define U1AckDelayTime 2

void UART1_INIT(void);
void UART1_RX_RXNE(void);
void Send_char(unsigned char ch);
void Send_String(unsigned char *string);
void Send_Data(unsigned char *P_data, unsigned int length);
void UART1_end(void);
void PC_PRG(void);

#endif
