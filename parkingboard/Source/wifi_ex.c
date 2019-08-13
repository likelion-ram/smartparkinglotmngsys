#include "device_driver.h"

unsigned char* at_cmd[]={
	"AT\r",
	"AT+WSET=0,iot24\r",
	"AT+WSEC=0,WPA2,123456789\r",
	"AT+WNET=1\r",
	"AT+WJOIN\r",
	"AT+SCON=SO,TCN, 192.168.0.7,5001 , ,0\r",	
	"AT+SSEND=0,,,5\rHello\r",
	"AT+FPING=3,192.168.0.1\r"
};

extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;
unsigned char cmd_buf[100];

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
        Uart1_Printf("n. next\n");
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
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
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
	
	for(i=0;i<n;i++)
	{
		if((cmd_buf[i]<'0')||(cmd_buf[i]>'9'))
		{
			return -1;
		}
		else
		{
			ret = ret*10 + cmd_buf[i]-'0';
		}
	}
	if(ret < (sizeof(at_cmd)/sizeof(at_cmd[0])))	return ret;
	else	return -1;
}

void WizFi250_Serial_Input_Mode(void)
{
	int index=0;
	int ret=0;

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
                        else if(Uart1_Rx_Data=='n'){
                          return;
                        }
			else
			{
				cmd_buf[index++] = Uart1_Rx_Data;
				Uart1_Send_Byte(Uart1_Rx_Data);
			}
		}
	}
}

