#include "device_driver.h"
#include "string.h"
#include "st7735.h"
#include "stdio.h"

#define	RES_OK		0
#define	RES_ERROR	1
#define	RES_CONN	2
#define	RES_DISCONN	3
#define	RES_LINKUP	4
#define	RES_LINKDN	5
#define	RES_VER		6

#define	MAX_ERR		3

unsigned char* at_cmd[]={
	"AT\r",
	"AT+WSET=0,iot24\r",
	"AT+WSEC=0,WPA2AES,123456789\r",
	"AT+WNET=1\r",
	"AT+WJOIN\r",
	"AT+SCON=SO,TCN,192.168.0.7,5001,,1\r",	
	"AT+SSEND=0,,,5\rHello\r",
	"AT+FPING=3,192.168.0.1\r"
};

const char* wait_msg[]={
	"[OK]\r\n",
	"[ERROR]\r\n",
	"[CONNECT 0]\r\n",
	"[DISCONNECT 0]\r\n",
	"[Link-Up Event]\r\n",
	"[Link_Down Event]\r\n",
	"(WIZnet Co.Ltd)\r\n",
};

int wait_msg_index[]={6, 9, 13, 16, 17, 19, 17};

extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;
extern volatile int wifi_mode;

unsigned char cmd_buf[120];

void Disp_Help(void)
{
	Uart1_Printf("\n < WIFI-CMD Help > \n");
	Uart1_Printf("0. Terminal Check\n");
	Uart1_Printf("1. WiFi Configuration \n");
	Uart1_Printf("2. WiFi Security \n");
	Uart1_Printf("3. Network Configuration \n");
	Uart1_Printf("4. WiFi Association \n");
	Uart1_Printf("5. Socket Open/Connect \n");
	Uart1_Printf("6. \"Hello\" Data Send \n");
	Uart1_Printf("7. Ping to Gateway\n");
}

void Factory_Default(void)  
{
#if 1         
	TIM_Delay_ms(100);
	W_Func_Low();
	TIM_Delay_ms(100);
	W_Func_High();
	TIM_Delay_ms(100);
	W_Func_Low();
	TIM_Delay_ms(100);
	W_Func_High(); 
	TIM_Delay_ms(100);
	W_Func_Low();
	TIM_Delay_ms(100);
	W_Func_High();
	TIM_Delay_ms(1500);
#endif  

	W_Rst_Low();
	TIM_Delay_ms(100);
	W_Rst_High();
	TIM_Delay_ms(1500);
}   
//--------------------------------------------------

void WiFi_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// clock configure SPI2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);	


	/* Configure SPI2 pins: SCK, MISO and MOSI ---------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// W_CSn, W_Func
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9  | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//W_RST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	W_CS_High();
	W_Rst_High();
	W_Func_High();

	/* SPI2 Config -------------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	// SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	//SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	      
	/* Enable SPI2 */
	SPI_Cmd(SPI2, ENABLE);
        
}
    

int Check_Cmd(int n)
{
	int i, ret=0;

	if((cmd_buf[0]<'0')||(cmd_buf[0]>'9'))
	{
		return -1;
	}
	
	for(i=n-1;i>=0;i--)
	{
		if((cmd_buf[i]<'0')||(cmd_buf[i]>'9'))
		{
			return ERROR;
		}
		else
		{
			ret = ret*10 + cmd_buf[i]-'0';
		}
	}
	return ret;
}


extern volatile int Uart2_Cmd_In;
extern volatile unsigned char Uart2_Rx_Buf[200];
extern volatile int index2;

int	Wait_Res(int n)
{
	for(;;)
	{
		if(Uart2_Cmd_In==1)
		{
			Uart2_Cmd_In = 0;
			if(!strncmp ((const char*)(Uart2_Rx_Buf+index2-wait_msg_index[n]), wait_msg[n],strlen(wait_msg[n])))
			{
				index2=0;
				return 0;
			}
			else
			{
				index2=0;
				return -1;
			}
		}
	}
}

int	Run_Server(void)
{
	int i=0, ret=0, err_cnt=0;

	Uart2_Send_String(at_cmd[0]);

	Wait_Res(RES_OK);

	Wait_Res(RES_VER);

	while(i<6)
	{
		Uart1_Printf("Step %d \n",i);
		Uart2_Send_String(at_cmd[i]);

		if(i==4)
		{
			ret = Wait_Res(RES_LINKUP);
			if(ret != NO_ERROR)
			{
				err_cnt++;
				if(err_cnt > MAX_ERR) return 0;
				continue;	
			}
		}
	
		ret = Wait_Res(RES_OK);
		if(ret == NO_ERROR)
		{
			i++;
		}
		else
		{
			Uart1_Printf("Step %d Response Error.. Retry..\n",i);
			TIM_Delay_ms(10);
			continue;
		}
		TIM_Delay_ms(10);
	}

	Uart1_Printf("Wait for client...");
	Wait_Res(RES_CONN);

	return 1;
}

int cnt=0;
char id[1];
void WizFi250_Serial_Data_Mode(void)
{
	int index=0;
	int ret=0;

	if(wifi_mode)
	{	
		Uart1_Printf("Enter Data Mode.. \n\n[send]: ");
		for(;;)
		{
			if(Uart2_Cmd_In)
                          
			{
                                int place1=1; //true 자리 있음
                                int place2=1;
				Uart2_Cmd_In = 0;
                                char out[10];
			        char print_price[8];
                                if(Uart2_Rx_Buf[0] == 'I') {
//                                  if(Uart2_Rx_Buf[4]=='1') place1=0;
//                                  if(Uart2_Rx_Buf[4]=='2') place2=0;
                                  /*
                                  if(cnt == 0) {    //있던 차가 0대 일 때
                                  cnt++;
                                    Lcd_Draw_Bar(0, 0, 80, 65, RED);
                                  }
                                  else if(cnt == 1) { //있던 차가 1대 일 때
                                    cnt++;
                                    Lcd_Draw_Bar(0, 65, 80, 130, RED);
                                  }
                                  */
                                  if(Uart2_Rx_Buf[3]=='1') {    //있던 차가 0대 일 때
                                  cnt++;
                                    Lcd_Draw_Bar(0, 0, 80, 65, RED);
                                  }
                                  else// if(Uart2_Rx_Buf[3]=='2') { //있던 차가 1대 일 때
                                  {  cnt++;
                                    Lcd_Draw_Bar(0, 65, 80, 130, RED);
                                  }
                                }
                                else { 
                                  //Lcd_Printf(100, 50, BLACK,WHITE, 1, 1, (char*)Uart2_Rx_Buf);
                                  strcpy(out, (char*)Uart2_Rx_Buf);
                                  strncpy(id, (char*)out, 1);
                                  strncpy(print_price, (char*)out+2, 7);
                                  
                                  Lcd_Printf(100, 50, BLACK,WHITE, 1, 1, print_price);
                                  
                                }
			}
			if(Uart1_Rx_In)
			{
				Uart1_Rx_In = 0;

				if(Uart1_Rx_Data == '\b')
				{
					Uart1_Printf("\b \b");
					index--;
				}
				else if(Uart1_Rx_Data == '\r')
				{
					cmd_buf[index] = 0x00;
					Uart2_Send_String(cmd_buf);
					Uart1_Printf("\n[send]: ");			
					index = 0;
				}
				else
				{
					cmd_buf[index++] = Uart1_Rx_Data;
					Uart1_Send_Byte(Uart1_Rx_Data);
				}
			}
		}
	}
	else
	{
		Uart1_Printf("Enter Command Mode.. \n\n");
		for(;;)
		{
			if(Uart1_Rx_In)
			{
				Uart1_Rx_In = 0;

				if(Uart1_Rx_Data == '\b')
				{
					Uart1_Printf("\b \b");
					index--;
				}
				else if(Uart1_Rx_Data == '\r')
				{
					Uart1_Printf("\n");

					if(index == 0)
					{
						Disp_Help();
					}
					else
					{
						ret = Check_Cmd(index);
						if(ret == -1)
						{
							cmd_buf[index++] = 0x0d;
							cmd_buf[index] = 0x00;
							Uart2_Send_String(cmd_buf);
						}
						else
						{
							Uart2_Send_String(at_cmd[ret]);
						}
						index = 0;
					}
				}
				else
				{
					cmd_buf[index++] = Uart1_Rx_Data;
					Uart1_Send_Byte(Uart1_Rx_Data);
				}
			}
		}
	}
}

