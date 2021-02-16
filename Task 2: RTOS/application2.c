#include "main.h"

#include "application.h"
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os.h"

#define PERIOD_CTRL 10
#define PERIOD_REF 4000

/* Function Prototypes */
void callback(void const *param);
void thread_1 (void const *argument);
void thread_2 (void const *argument);

/* Thread setup */
osThreadId main_ID, thread_1_ID, thread_2_ID;
osThreadDef(thread_1, osPriorityNormal, 1, 0);
osThreadDef(thread_2, osPriorityNormal, 1, 0);

/* Timer setup */
osTimerDef(timer_1_ID, callback);
osTimerDef(timer_2_ID, callback);


/* Global variables ----------------------------------------------------------*/
int16_t encoder;
int32_t reference, velocity, control;
uint32_t millisec;

/* Functions -----------------------------------------------------------------*/


/* Define Thread 1,  */
void thread_1()
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		
		// Get time
		millisec = SysTick_ms();
		
		// Get current velocity
		encoder = Peripheral_Timer_ReadEncoder();
		velocity = Controller_CalculateVelocity(encoder, millisec);

		// Calculate control signal
		control = Controller_PIController(reference, velocity, millisec);

		// Apply control signal to motor
		Peripheral_PWM_ActuateMotor(control);
	}

}


/* Define Thread 2,  */
void thread_2()
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		reference = -reference;
	}
}

/* Define callback function for thread wakeup */
void callback(void const *param)
{
	switch( (uint32_t) param)
	{
		case 0:
			osSignalSet(thread_1_ID, 0x01);
		break;

		case 1:
			osSignalSet(thread_2_ID, 0x01);
		break;
	}
}
/* Run setup needed for all periodic tasks */
int Application_Setup()
{
	// Reset global variables
	reference = 2000;
	velocity = 0;
	control = 0;
	millisec = 0;

	// Initialise hardware DONE
	Peripheral_GPIO_EnableMotor();
	
	// Initialise timers
	osTimerId timer_1 = osTimerCreate(osTimer(timer_1_ID), osTimerPeriodic, (void *)0);	
	osTimerId timer_2 = osTimerCreate(osTimer(timer_2_ID), osTimerPeriodic, (void *)1);	
	
	// initialize CMSIS-RTOS
	osKernelInitialize ();

	// Start timers
	osTimerStart(timer_1, PERIOD_CTRL);	
	osTimerStart(timer_2, PERIOD_REF);
	
	// Start threads
	thread_1_ID = osThreadCreate(osThread(thread_1), NULL);
	thread_2_ID = osThreadCreate(osThread(thread_2), NULL);
	
  // start thread execution 
	osKernelStart ();    
	

	return 0;
}

/* Define what to do in the infinite loop */
void Application_Loop()
{	
	
	// Do Nothing
	osSignalWait(0x01, osWaitForever);
	
}
