#include "stm32f10x.h"
#include "macro.h"

extern void Uart1_Printf(char *fmt,...);


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
void TIM2_Init(void);
void TIM4_Init(void);

/*
TIM2 : interrupt 사용 (LED, 기타 플래그 )
TIM3 : 기타 딜레이 함수에 사용
TIM4 : 온도센서 딜레이 추가에 사용(자간딜레이)
*/
void TIM_Config(void)
{
/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM2_Init();
//	TIM4_Init();
}

void TIM2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	uint16_t PrescalerValue = 0;
	
  /* Enable the TIM2 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	

	
  /* ---------------------------------------------------------------------------
	TIM2 Configuration: TIM_CounterMode_Down:
	TIM2CLK = 72 MHz, Prescaler = 7199, TIM2 counter clock = 10 KHz, Period = 9
  ----------------------------------------------------------------------------*/
	
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 9;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	
		/* Compute the prescaler value */
//	PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 10000) - 1;
	PrescalerValue = (uint16_t) ( (SystemCoreClock) / 10000) - 1;
	
	
	/* Prescaler configuration */
  TIM_PrescalerConfig(TIM2, PrescalerValue, TIM_PSCReloadMode_Immediate);
	
	 /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	
	
  /* TIM enable counter */
  TIM_Cmd(TIM2, ENABLE);
	
 
}


void TIM4_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	uint16_t PrescalerValue = 0;
	
  /* Enable the TIM4 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	
	
  /* ---------------------------------------------------------------------------
	TIM4 Configuration: TIM_CounterMode_Down:
	TIM4 CLK = 72 MHz, Prescaler = 7199, TIM2 counter clock = 10 KHz, Period = 9
  ----------------------------------------------------------------------------*/
	
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 9;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) ( (SystemCoreClock) / 10000) - 1;
	
	
	/* Prescaler configuration */
  TIM_PrescalerConfig(TIM4, PrescalerValue, TIM_PSCReloadMode_Immediate);
	
	
  /* TIM enable counter */
  TIM_Cmd(TIM4, ENABLE);
	
  /* TIM IT enable */
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}



#define TIM3_CLK    72000000

#define TIM3_TICK_us   100               // 100 ns
#define TIM3_FREQ_us  (1000000000/TIM3_TICK_us)
#define TIM3_PLS_OF_1us	(1000/TIM3_TICK_us)

#define TIM3_TICK_ms   20                      // usec
#define TIM3_FREQ_ms  (1000000/TIM3_TICK_ms)
#define TIM3_PLS_OF_1ms	(1000/TIM3_TICK_ms)

void TIM_Delay_us(unsigned int time)
{
    int i;
	unsigned int t = TIM3_PLS_OF_1us * time;
	
    //TIM3 Enable
    Peri_Set_Bit(RCC->APB1ENR, 1);
    
    TIM3->PSC = (unsigned int)(TIM3_CLK/(double)TIM3_FREQ_us + 0.5)-1;
    TIM3->CR1 = (1<<4)+(1<<3)+(1<<2);    
    
    
    ////////////
    Peri_Set_Bit(TIM3->EGR,0);
	Peri_Set_Bit(TIM3->DIER, 0);

	for(i=0; i<(t/0xffff); i++)
	{
		TIM3->CNT = 0xffff;
		Peri_Clear_Bit(TIM3->SR, 0);
		Peri_Set_Bit(TIM3->CR1, 0);
		while(Macro_Check_Bit_Clear(TIM3->SR, 0));
	}

	TIM3->CNT = t% 0xffff;
	Peri_Clear_Bit(TIM3->SR, 0);
	Peri_Set_Bit(TIM3->CR1, 0);
	while (Peri_Check_Bit_Clear(TIM3->SR, 0));


	Peri_Clear_Bit(TIM3->CR1, 0);
	Peri_Clear_Bit(TIM3->DIER, 0);
	Peri_Clear_Bit(RCC->APB1ENR, 1);
 
}

void TIM_Delay_ms(unsigned int time)
{
    volatile int i;
	unsigned int t = TIM3_PLS_OF_1ms * time;
	
    //TIM3 Enable
    Peri_Set_Bit(RCC->APB1ENR, 1);
    
    TIM3->PSC = (unsigned int)(TIM3_CLK/(double)TIM3_FREQ_ms + 0.5)-1;
    TIM3->CR1 = (1<<4)+(1<<3)+(1<<2); 
    
    
    ////////////
    Peri_Set_Bit(TIM3->EGR,0);
	Peri_Set_Bit(TIM3->DIER, 0);

	for(i=0; i<(t/0xffff); i++)
	{
		TIM3->CNT = 0xffff;
		Peri_Clear_Bit(TIM3->SR, 0);
		Peri_Set_Bit(TIM3->CR1, 0);
		while(Macro_Check_Bit_Clear(TIM3->SR, 0));
	}

	TIM3->CNT = t% 0xffff;
	Peri_Clear_Bit(TIM3->SR, 0);
	Peri_Set_Bit(TIM3->CR1, 0);
	while (Peri_Check_Bit_Clear(TIM3->SR, 0));


	Peri_Clear_Bit(TIM3->CR1, 0);
	Peri_Clear_Bit(TIM3->DIER, 0);
	Peri_Clear_Bit(RCC->APB1ENR, 1);
 
}

void TIM_Stopwatch_Start(void)
{
	Peri_Set_Bit(RCC->APB1ENR, 1);

	TIM3->CR1 = (1<<4)+(1<<3);
	TIM3->PSC = (unsigned int)(TIM3_CLK/(double)TIM3_FREQ_ms + 0.5)-1;
	TIM3->CNT = 0xffffu;
	Peri_Set_Bit(TIM3->EGR,0);
	Peri_Set_Bit(TIM3->CR1, 0);
}

unsigned int TIM_Stopwatch_Stop(void)
{
	unsigned int time;

	Peri_Clear_Bit(TIM3->CR1, 0);
	time = (0xffffu - TIM3->CNT) * TIM3_TICK_ms;
	Peri_Clear_Bit(RCC->APB1ENR, 1);
	return time;
}
