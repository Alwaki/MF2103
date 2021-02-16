#include "main.h"

#include "application.h"
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os.h"

#define PERIOD_CTRL 10
#define PERIOD_REF 4000

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
		osDelay(10);
		
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
		osDelay(4000);
		reference = -reference;
	}
}

// Define the thread handles and thread parameters
osThreadId main_ID, thread_1_ID, thread_2_ID;
osThreadDef(thread_1, osPriorityNormal, 1, 0);
osThreadDef(thread_2, osPriorityNormal, 1, 0);

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
	
	// initialize CMSIS-RTOS
	osKernelInitialize ();
	
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
	
	/* COMMENT OUT OLD CODE
	
	// Get time
		millisec = SysTick_ms();

		// Every 10 msec ...
		if (millisec % PERIOD_CTRL == 0)
		{
			// Get current velocity DONE
			encoder = Peripheral_Timer_ReadEncoder();
			velocity = Controller_CalculateVelocity(encoder, millisec);

			// Calculate control signal
			control = Controller_PIController(reference, velocity, millisec);

			// Apply control signal to motor
			Peripheral_PWM_ActuateMotor(control);

			// Every 4 sec ...
			if (millisec % PERIOD_REF == 0)
			{
				// Flip the direction of the reference
				reference = -reference;
			}

			// Prevent from re-entering again
			while(SysTick_ms() % PERIOD_CTRL == 0);
		}
		*/
}
