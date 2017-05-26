/***********************************************************************/
/*  FILE        :ID_Decode.c                                           */
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
#include "ID_Decode.h"
#include "eeprom.h"		// eeprom
#include "uart.h"		// uart
#include "adf7021.h"		// 初始化ADF7021

void EXIT_init(void){
   EXTI_CR1=0x20;             //PORT B2  的中断触发位
   ADF7021_DATA_CLK_CR2=1;     //使能该I/O口中断  PA1
//   EXTI_CR2=0X00;   
//   PIN_PD7_CR2=1;      
}


void ID_Decode_function(void)
{
  UINT16 DATA_Packet_Syn_bak=0;
  if(FLAG_APP_RX==1){
     TIME_EMC=10;
     switch (rxphase){
        case 0:
                DATA_Packet_Syn=DATA_Packet_Syn<<1;
                if(ADF7021_DATA_rx)DATA_Packet_Syn+=1;
                if(TIMER18ms==0){
                                 DATA_Packet_Syn_bak=DATA_Packet_Syn&0x0000FFFF;
                                 if((DATA_Packet_Syn_bak==0x5555)||(DATA_Packet_Syn_bak==0xAAAA));
                                 else FLAG_Receiver_Scanning=1;
                                }
                //if(DATA_Packet_Syn==0x55555555){rxphase=1;TIMER18ms=65;DATA_Packet_Syn=0;DATA_Packet_Head=0;}
                if((DATA_Packet_Syn&0xFFFFFFFF)==0x55555555){
		    rxphase=1;TIMER18ms=2000;DATA_Packet_Syn=0;DATA_Packet_Head=0;
                    //Receiver_LED_RX=1;
		    FG_Receiver_LED_RX=1;
                    TIMER300ms=500; 
                   }
                break;
	case 1:
                DATA_Packet_Head=DATA_Packet_Head<<1;
                if(ADF7021_DATA_rx)DATA_Packet_Head+=1;
                //DATA_Packet_Head=DATA_Packet_Head&0x0000FFFF;
                if(TIMER18ms==0)rxphase=0;
                if(DATA_Packet_Head==0x5515){rxphase=2;DATA_Packet_Syn=0;DATA_Packet_Head=0;DATA_Packet_Code_i=0;}
		break;
        case 2:
                DATA_Packet_Code_g=DATA_Packet_Code_i/32;
                DATA_Packet_Code[DATA_Packet_Code_g]=DATA_Packet_Code[DATA_Packet_Code_g]<<1;
                if(ADF7021_DATA_rx)DATA_Packet_Code[DATA_Packet_Code_g]+=1;
                DATA_Packet_Code_i++;
                if(DATA_Packet_Code_i==96){
                    if((DATA_Packet_Code[1]&0x0000FFFF)==0x5556);
                    else rxphase=3;
                }
                else if(DATA_Packet_Code_i>=192)rxphase=3;
                break;
        case 3:
                FLAG_Receiver_IDCheck=1;
//                if((Freq_Scanning_CH==1)||(Freq_Scanning_CH==3)||(Freq_Scanning_CH==5))Freq_Scanning_CH_bak=0;   //暂时记录下收到信号的频率信道,0代表426M
//                else Freq_Scanning_CH_bak=1;                                                                                 //                       1代表429M
                rxphase=0;
                DATA_Packet_Syn=0;
                TIMER18ms=0;   //0ms，接受可靠稳定
                break;
        default:
               break;
	}
  }
  else if(FG_test_rx==1){
      X_COUNT ++;
      if((ADF7021_DATA_rx== X_HIS)&&(X_COUNT != 1))X_ERR++;
      X_HIS = ADF7021_DATA_rx;
  }
  EXTI_SR1_bit.P2F=1;
}



void ID_Decode_IDCheck(void)
{
    if(FLAG_Receiver_IDCheck)
    {
        FLAG_Receiver_IDCheck=0;
        Signal_DATA_Decode(0);
        if(FLAG_Signal_DATA_OK==1)
        {
            eeprom_IDcheck();
            if((FLAG_ID_Erase_Login==1)||(FLAG_ID_Login==1)){
                if((FLAG_ID_Login_OK==0)&&(DATA_Packet_Contro_buf!=0x40)&&(DATA_Packet_ID!=0))    //2015.4.1修正 在登录模式下 不允许自动送信登录，只允许手动送信登录
		{FLAG_ID_Login_OK=1;ID_Receiver_Login=DATA_Packet_ID;}
            }
            else if((FLAG_IDCheck_OK==1)||(DATA_Packet_ID==0xFFFFFE))
            {
                FLAG_IDCheck_OK=0;
                if(DATA_Packet_ID==0xFFFFFE)DATA_Packet_Control=DATA_Packet_Contro_buf;      //2015.3.24修正 Control缓存起 ID判断是否学习过后才能使用
//                if(Freq_Scanning_CH_bak==0){Freq_Scanning_CH_save=1;Freq_Scanning_CH_save_HA=0; }  //当前收到426M控制   但保存记录下收到信号的频率信道,0代表426M
//                else Freq_Scanning_CH_save_HA=1;  //                       1代表429M
//                DATA_Packet_Control_0=DATA_Packet_Control;
//             #if defined(__Product_PIC32MX2_Receiver__)
//                if(DATA_Packet_Control==0x08)DATA_Packet_Control_err=0x08;
//                if(DATA_Packet_Control==0x02){DATA_Packet_Control_err=0x02;FLAG_HA_ERR_bit=0;}
//             #endif
//                if(((DATA_Packet_Code[1]&0x0000FFFF)==0x5556)&&(Freq_Scanning_CH_bak==0)){
		if((DATA_Packet_Code[1]&0x0000FFFF)==0x5556){
                    Signal_DATA_Decode(1);
                    if(FLAG_Signal_DATA_OK==1){
                            eeprom_IDcheck();
                            if(DATA_Packet_Contro_buf==0xFF){                   //2015.4.2修正  送信器旁边的登录键 追加登录不行
                                if(FLAG_IDCheck_OK==1)FLAG_IDCheck_OK=0;
                                 else if(ID_DATA_PCS<256){
                                     BEEP_and_LED();
                                     ID_Receiver_Login=DATA_Packet_ID;
                                     ID_EEPROM_write();
                                 }//end else
                            }
                            else if(DATA_Packet_Control==0x00){
                                if(FLAG_IDCheck_OK==1){
                                     FLAG_IDCheck_OK=0;
                                     BEEP_and_LED();
                                     ID_EEPROM_write_0x00();
                                 }
                            }
                    }
                }
                else
                   {
//#if defined(__Product_PIC32MX2_WIFI__)
//                    TIMER1s=500;//1000
//#endif
//#if defined(__Product_PIC32MX2_Receiver__)
//                    if(Freq_Scanning_CH_bak==0){
                        if((DATA_Packet_Control==0x40)&&(Manual_override_TIMER==0)){
                          FG_auto_manual_mode=1;
                          TIME_auto_out=890;    // 900
                          if(FG_First_auto==0){
                              FG_First_auto=1;
                              TIMER1s=3000;    //2500 
                          }
                        }       
                        else if(DATA_Packet_Control==0x40);
			else{
			   FG_auto_out=0;
			   TIME_auto_close=0;
                           FG_auto_open_time=0;
                           if(FG_auto_manual_mode==1)//Manual_override_TIMER=13500;   //2分30秒自动无效
                              Manual_override_TIMER=24480;   //4分30秒自动无效
		           if((DATA_Packet_Control&0x14)==0x14){if(TIMER1s==0)TIMER1s=3800-30;}
			   else  TIMER1s=1000;
			}
//                    }
//                    else TIMER1s=1000;
                    TIMER300ms=500;
                    //Receiver_LED_RX=1;
		    FG_Receiver_LED_RX=1;
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
    UINT8 i,j;
        for(i=0;i<3;i++)
        {
            if(NUM_Type==0)data_in=DATA_Packet_Code[i];
            else data_in=DATA_Packet_Code[i+3];
            data_out=0;
            data_in=data_in>>1;
            for(j=0;j<16;j++)
            {
                data_out=data_out<<1;
                if(data_in&0x00000001)data_out+=1;
                data_in=data_in>>2;
            }
           data_NRZ[i] =data_out;
        }
      if(data_NRZ[2]==((data_NRZ[0]+data_NRZ[1])&0xFFFF)){
          FLAG_Signal_DATA_OK=1;
          DATA_Packet_ID=(data_NRZ[1]&0x00FF)*65536+data_NRZ[0];
          if(DATA_Packet_ID==0)FLAG_Signal_DATA_OK=0;    //2014.3.21追加  不允许使用ID=0
          DATA_Packet_Contro_buf=(data_NRZ[1]&0xFF00)>>8;  //2015.3.24修正 Control缓存起 ID判断是否学习过后才能使用 
//          if(Freq_Scanning_CH_bak==1)
//              Control_bak=DATA_Packet_Control;
      }
      else FLAG_Signal_DATA_OK=0;
}

void eeprom_IDcheck(void)
{
    UINT16 i;
   for(i=0;i<ID_DATA_PCS;i++){
       if(ID_Receiver_DATA[i]==DATA_Packet_ID){INquiry=i;i=ID_DATA_PCS;FLAG_IDCheck_OK=1;DATA_Packet_Control=DATA_Packet_Contro_buf;}      //2015.3.24修正 Control缓存起 ID判断是否学习过后才能使用
       if((FLAG_ID_Erase_Login==1)&&(FLAG_ID_Erase_Login_PCS==1)){i=ID_DATA_PCS;FLAG_IDCheck_OK=0;DATA_Packet_Control=DATA_Packet_Contro_buf;}         //追加多次ID登录
   }
}


void BEEP_and_LED(void)
{
   UINT16 i;
// #if defined(__Product_PIC32MX2_Receiver__)
     Receiver_LED_OUT=1;
     for(i=0;i<4160;i++){
         //Receiver_Buzzer=!Receiver_Buzzer;   //蜂鸣器频率2.08KHZ
         if(FG_beep_on==0){FG_beep_on=1;FG_beep_off=0;BEEP_CSR2_BEEPEN=1;}
         //Delayus(240);
	 Delayus(250);   //80us
	 Delayus(250);   //80us
	 Delayus(250);   //80us
         ClearWDT(); // Service the WDT
     }
     //Receiver_Buzzer=0;
     FG_beep_on=0;
     BEEP_CSR2_BEEPEN=0;     
     //Receiver_LED_OUT=0;
     TIME_Receiver_LED_OUT=185;
//#endif
//#if defined(__Product_PIC32MX2_WIFI__)
//     WIFI_LED_RX=1;
//     for(i=0;i<8000;i++){        
//         Delayus(190);       //2.08KHZ
//         ClearWDT(); // Service the WDT
//     }
//     WIFI_LED_RX=0;
//#endif
}


void Receiver_BEEP(void)
{
   UINT16 i,j;
   if(FLAG_Receiver_BEEP==0)
   {
       FLAG_Receiver_BEEP=1;
       for(j=0;j<3;j++){
         for(i=0;i<1800;i++){
             //Receiver_Buzzer=!Receiver_Buzzer;   //蜂鸣器频率2.08KHZ
	     if(FG_beep_on==0){FG_beep_on=1;FG_beep_off=0;BEEP_CSR2_BEEPEN=1;}
            //Delayus(240);
	     Delayus(250);   //80us
	     Delayus(250);   //80us
	     Delayus(250);   //80us
             ClearWDT(); // Service the WDT
         }
         for(i=0;i<1800;i++){
            // Receiver_Buzzer=0;   //蜂鸣器频率2.08KHZ
	     if(FG_beep_off==0){FG_beep_off=1;FG_beep_on=0;BEEP_CSR2_BEEPEN=0;}
            //Delayus(240);
	     Delayus(250);   //80us
	     Delayus(250);   //80us
	     Delayus(250);   //80us
             ClearWDT(); // Service the WDT
         }
       }
       //Receiver_Buzzer=0;
   }
}




void ID_Decode_OUT(void)
{
    UINT8 Control_i;
//    if(Freq_Scanning_CH_bak==0) Control_i=DATA_Packet_Control&0xFF;
//    else Control_i=DATA_Packet_Control&0x0E;
//    if(HA_Sensor_signal==1)Receiver_LED_TX=0;                      //test 接近信号回路
//     else Receiver_LED_TX=1;

    Control_i=DATA_Packet_Control&0xFF;
    if(TIMER1s){
                switch (Control_i){
                     case 0x14:                            //stop+login
                                Receiver_LED_OUT=1;
                                TIMER250ms_STOP=250;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_STOP=FG_allow_out;
                                if(TIMER1s<3550){Receiver_OUT_OPEN=FG_allow_out;
                                                 Receiver_OUT_CLOSE=FG_allow_out;Receiver_BEEP();}
                                break;
                     case 0x40:                            //自动送信
		                if((FG_auto_out==0)&&(Manual_override_TIMER==0)){
                                    Receiver_LED_OUT=1;
                                    TIMER250ms_STOP=0;
				    Receiver_OUT_VENT=FG_NOT_allow_out;
				    Receiver_OUT_CLOSE=FG_NOT_allow_out;
                                    if(TIMER1s>2000){Receiver_OUT_STOP=FG_allow_out;Receiver_OUT_OPEN=FG_NOT_allow_out;}   //1830
                                    else if(TIMER1s>1000){Receiver_OUT_STOP=FG_NOT_allow_out;Receiver_OUT_OPEN=FG_NOT_allow_out;}   //810
				    else {FG_auto_open_time=1;Receiver_OUT_STOP=FG_NOT_allow_out;Receiver_OUT_OPEN=FG_allow_out;}
                                    
		                }
                                break;
                     case 0x01:                              //VENT
                                Receiver_LED_OUT=1;
				if(Receiver_vent==0){         //受信机换气联动OFF
				    Receiver_OUT_STOP=FG_NOT_allow_out;
				    Receiver_OUT_VENT=FG_NOT_allow_out;
                                    Receiver_OUT_OPEN=FG_allow_out;
                                    Receiver_OUT_CLOSE=FG_allow_out;
				}
				else {                      //受信机换气联动ON
				   Receiver_OUT_STOP=FG_NOT_allow_out;
				   Receiver_OUT_OPEN=FG_NOT_allow_out;
				   Receiver_OUT_CLOSE=FG_NOT_allow_out;
				   Receiver_OUT_VENT=FG_allow_out;
				}
                                break;				
                     case 0x02:                              //close
                                Receiver_LED_OUT=1;
                                Receiver_OUT_OPEN=FG_NOT_allow_out;
                                Receiver_OUT_STOP=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_CLOSE=FG_allow_out;
                                break;
                     case 0x04:                           //stop
                                Receiver_LED_OUT=1;
                                Receiver_OUT_OPEN=FG_NOT_allow_out;
                                Receiver_OUT_CLOSE=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_STOP=FG_allow_out;
                                break;
                     case 0x08:                         //open
                                Receiver_LED_OUT=1;
                                Receiver_OUT_STOP=FG_NOT_allow_out;
                                Receiver_OUT_CLOSE=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_OPEN=FG_allow_out;
                                break;
                     case 0x0C:                         //open+stop
                                Receiver_LED_OUT=1;
                                TIMER250ms_STOP=250;
                                Receiver_OUT_CLOSE=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_STOP=FG_allow_out;
				if(FG_OUT_OPEN_CLOSE==0){FG_OUT_OPEN_CLOSE=1;TIME_OUT_OPEN_CLOSE=25;}
                                if(TIME_OUT_OPEN_CLOSE==0)Receiver_OUT_OPEN=FG_allow_out;
                                break;
                     case 0x06:                       //close+stop
                                Receiver_LED_OUT=1;
                                TIMER250ms_STOP=250;
                                Receiver_OUT_OPEN=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_STOP=FG_allow_out;
				if(FG_OUT_OPEN_CLOSE==0){FG_OUT_OPEN_CLOSE=1;TIME_OUT_OPEN_CLOSE=25;}
                                if(TIME_OUT_OPEN_CLOSE==0)Receiver_OUT_CLOSE=FG_allow_out;   
                                break;
                     case 0x0A:                       //close+OPEN
                                Receiver_LED_OUT=1;
                                Receiver_OUT_STOP=FG_NOT_allow_out;
				Receiver_OUT_VENT=FG_NOT_allow_out;
                                Receiver_OUT_OPEN=FG_allow_out;
                                Receiver_OUT_CLOSE=FG_allow_out;   
                                break;
                     case 0x09:                       //vent+OPEN
                                Receiver_LED_OUT=1;
                                Receiver_OUT_STOP=FG_NOT_allow_out;
				Receiver_OUT_CLOSE=FG_NOT_allow_out;
                                Receiver_OUT_OPEN=FG_allow_out;
                                Receiver_OUT_VENT=FG_allow_out;   
                                break;
                     case 0x03:                       //vent+close
                                Receiver_LED_OUT=1;
                                Receiver_OUT_STOP=FG_NOT_allow_out;
				Receiver_OUT_OPEN=FG_NOT_allow_out;
                                Receiver_OUT_CLOSE=FG_allow_out;
                                Receiver_OUT_VENT=FG_allow_out;   
                                break;
                     default:
                                break;
                 }
                //if((DATA_Packet_Control==0x00)&&(FLAG_APP_Reply==0)) FLAG_APP_Reply=1;
                //if(((DATA_Packet_Control==0x00)||(DATA_Packet_Control==0x02)||(DATA_Packet_Control==0x08))&&(FLAG_APP_Reply==0)&&(Freq_Scanning_CH_save_HA==1)) FLAG_APP_Reply=1;
        //        if(((DATA_Packet_Control==0x00)||(DATA_Packet_Control==0x02)||(DATA_Packet_Control==0x04)||(DATA_Packet_Control==0x08))&&(FLAG_APP_Reply==0)&&(Freq_Scanning_CH_save_HA==1)) FLAG_APP_Reply=1;
//                if((DATA_Packet_Control&0x14)==0x14){
//                    TIMER250ms_STOP=250;
//                    if(TIMER1s<3550){Receiver_OUT_OPEN=1;Receiver_OUT_CLOSE=1;Receiver_BEEP();}
//                }
//                if((DATA_Packet_Control&0x08)==0x08)Receiver_OUT_OPEN=1;
//                if((DATA_Packet_Control&0x04)==0x04)Receiver_OUT_STOP=1;
//                if((DATA_Packet_Control&0x02)==0x02)Receiver_OUT_CLOSE=1;
//                if((DATA_Packet_Control&0x0C)==0x0C)TIMER250ms_STOP=250;
//                if((DATA_Packet_Control&0x06)==0x06)TIMER250ms_STOP=250;
          }       
     else {
//           if(FLAG_APP_Reply==1){FLAG_APP_Reply=0;ID_data.IDL=DATA_Packet_ID;Control_code=HA_Status;FLAG_HA_START=1;}
//           if(FLAG_426MHz_Reply==1){FLAG_426MHz_Reply=0;ID_data.IDL=DATA_Packet_ID;Control_code=HA_Status+4;FLAG_HA_START=1;}   //受信器自动发送HA状态码为实际HA码+4
           if((FG_auto_out==1)&&(TIME_auto_out==0)){FG_auto_out=0;TIME_auto_close=270;Receiver_LED_OUT=1;}   //300
	   if(TIME_auto_close){
                if(TIME_auto_close>180){Receiver_OUT_STOP=FG_allow_out;Receiver_OUT_CLOSE=FG_NOT_allow_out;}  //200
                else if(TIME_auto_close>90){Receiver_OUT_STOP=FG_NOT_allow_out;Receiver_OUT_CLOSE=FG_NOT_allow_out;}   //100
	        else {Receiver_OUT_STOP=FG_NOT_allow_out;Receiver_OUT_CLOSE=FG_allow_out;}	     
	   }
	   else   {FG_auto_manual_mode=0;Receiver_OUT_CLOSE=FG_NOT_allow_out;}
           FG_First_auto=0;
           FLAG_Receiver_BEEP=0;
           if((FLAG_ID_Erase_Login==1)||(FLAG_ID_Login==1)||(TIME_auto_close));
           else if(TIME_Receiver_LED_OUT>0)Receiver_LED_OUT=1;
	   else Receiver_LED_OUT=0;
           Receiver_OUT_OPEN=FG_NOT_allow_out;
	   Receiver_OUT_VENT=FG_NOT_allow_out;
           if(FG_auto_open_time==1){FG_First_auto=0;FG_auto_out=1;FG_auto_open_time=0;}
           if((TIMER250ms_STOP==0)&&(TIME_auto_close==0)){Receiver_OUT_STOP=FG_NOT_allow_out;FG_OUT_OPEN_CLOSE=0;}
          }
    if(TIMER300ms==0)FG_Receiver_LED_RX=0;   //Receiver_LED_RX=0;
}


void  Freq_Scanning(void)
{
//    //if((FLAG_Receiver_Scanning==1)&&(FLAG_APP_RX==1)&&(FLAG_UART_ok==0))
//    if(((FLAG_Receiver_Scanning==1)||(TIME_EMC==0))&&(FLAG_APP_RX==1)&&(FLAG_UART_ok==0))
//    {
//        FLAG_Receiver_Scanning=0;
//        Freq_Scanning_CH++;
//        if(Freq_Scanning_CH>6){Freq_Scanning_CH=1;dd_set_ADF7021_ReInitial();}
//        dd_set_ADF7021_Freq(0,Freq_Scanning_CH);
//
//
////        TIMER18ms=18;//18;
//        if((Freq_Scanning_CH==1)||(Freq_Scanning_CH==3)||(Freq_Scanning_CH==5))TIMER18ms=36;
//        else TIMER18ms=18;
//    }
  
  
    //if((FLAG_Receiver_Scanning==1)&&(FLAG_APP_RX==1)&&(FLAG_UART_ok==0))
//    if(((FLAG_Receiver_Scanning==1)||(TIME_EMC==0))&&(FLAG_APP_RX==1))
//    {
//        FLAG_Receiver_Scanning=0;
//        TIMER18ms=36;
//        dd_set_RX_mode();
//    }  
  
    if(((FLAG_Receiver_Scanning==1)||(TIME_EMC==0)||(TIME_Fine_Calibration==0))&&(FLAG_APP_RX==1))
    {
        FLAG_Receiver_Scanning=0;
        if(TIME_Fine_Calibration==0){TIME_Fine_Calibration=9000;dd_set_ADF7021_Power_on();dd_set_RX_mode();}
        dd_set_ADF7021_Freq();
        TIMER18ms=36;
    }  
}
