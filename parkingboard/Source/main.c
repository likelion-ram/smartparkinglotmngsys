#include "device_driver.h"

RCC_ClocksTypeDef RCC_ClockFreq;
extern volatile int ADC1_value;
extern volatile int ADC1_flag;
extern volatile int Flag_send;
extern volatile int TIM4_Expired;
extern void WizFi250_Serial_Input_Mode(void);

int main(void)
{
        float AD_value;
	RCC_GetClocksFreq(&RCC_ClockFreq);

	TIM_Config();
	Uart1_Init(115200);
	Uart2_Init(115200);
        LED_Init();
        ADC_Configuration();
	WiFi_Init();
        TIM4_Repeat_Interrupt_Enable(1, 500);
        WizFi250_Serial_Input_Mode();	

#if	0
	Factory_Default();
#endif
        LED_On();
	//Uart1_Printf("Wifi server command mode Test\n");
        
	for(;;){
	        //Uart1_Printf("in?\n");      
                if(TIM4_Expired)
		{
			TIM4_Expired = 0;
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		}

		if(ADC1_flag)
		{
			AD_value = (ADC1_value*3.3)/4096.;
			//if(AD_value < 0.5) Uart1_Printf("under 0.5v\n");
			ADC1_flag = 0;   
		}
                if(Flag_send)
                {
                        //Uart2_Send_String("AT+SSEND=0,,,5\rHello\r");
                        //Uart1_Printf("aa\n");
                        Flag_send=0;
                }
        
        }
	
}
