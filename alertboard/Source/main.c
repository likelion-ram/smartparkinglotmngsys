#include "device_driver.h"
#include "st7735.h"

RCC_ClocksTypeDef RCC_ClockFreq;

extern void WizFi250_Serial_Input_Mode(void);
extern volatile int wifi_mode;

int main(void)
{
	RCC_GetClocksFreq(&RCC_ClockFreq);

	TIM_Config();
	Uart1_Init(115200);
	Uart2_Init(115200);
        
        Key_ISR_Enable(1);
        Lcd_Init();
	WiFi_Init();

	Factory_Default();
	wifi_mode = Run_Server();
        
        Lcd_Draw_Bar(0, 0, 80, 130, GREEN);
        Lcd_Draw_Bar(80, 0, 160, 130, WHITE);
        
	Uart1_Printf("Wifi server command mode Test\n");

	for(;;)
	{
		WizFi250_Serial_Data_Mode();
	}
}
