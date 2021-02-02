#include "stm32l4xx.h"
#include "peripherals.h"

/* Enable both half-bridges to drive the motor */
void Peripheral_GPIO_EnableMotor(void)
{
	//Set pin 5&6 in port A to 0b01 (high)
	GPIOA -> BSRR = GPIO_BSRR_BS5;
	GPIOA -> BSRR = GPIO_BSRR_BS6;
}

/* Drive the motor in both directions */
void Peripheral_PWM_ActuateMotor(int16_t control)
{
	uint32_t DC;
	DC = abs(control);
	
	if(control > 0) 
	{
		//negative direction
		TIM3 -> CCR2 = 0;
		TIM3 -> CCR1 = DC;
	}
	else
	{
		//positive direction
		TIM3 -> CCR1 = 0;
		TIM3 -> CCR2 = DC;
	}

	return;
}

/* Read the counter register to get the encoder state */
int16_t Peripheral_Timer_ReadEncoder(void)
{
	int16_t counter;
	counter = TIM1 -> CNT;
	TIM1 -> CNT = 0;
	return counter;
}
