#include "main.h"
#include <stdio.h>
#include "application.h"
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os.h"
#include "socket.h"

#define PERIOD_CTRL 10
#define PERIOD_REF 4000
#define APP_SOCK 0
#define SERVER_PORT 2103
#define CLIENT_PORT 2104

/* Function Prototypes */
void callback(void const *param);
void thread_sample (void const *argument);
void thread_actuate (void const *argument);
void thread_client (void const *argument);

/* Thread setup */
osThreadId main_ID, thread_sample_ID, thread_actuate_ID, thread_client_ID;
osThreadDef(thread_sample, osPriorityNormal, 1, 0);
osThreadDef(thread_actuate, osPriorityNormal, 1, 0);
osThreadDef(thread_client, osPriorityBelowNormal, 1, 0);


/* Timer setup */
osTimerDef(timer_sample_ID, callback);

/* Global variables ----------------------------------------------------------*/
int16_t encoder;
int32_t reference, velocity, control, stop_signal = 0;
int32_t velocity_boosted, control_boosted;
uint32_t uvelocity, ucontrol;
uint32_t millisec;
uint8_t returnvalue;
uint8_t connected = 0;

uint8_t server_addr[4] = {192, 168, 0, 10};
uint8_t retval;
uint8_t sock_status;

/* Functions -----------------------------------------------------------------*/

/* Define bootup sequence */
void bootup()
{
 printf("Opening socket... ");
 if((retval = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK)
 {
 printf("Success!\n\r");
 // Try to connect to server
 printf("Connecting to server... ");
 if((retval = connect(APP_SOCK, server_addr, SERVER_PORT)) == SOCK_OK)
 {
 printf("Success!\n\r");
 retval = getsockopt(APP_SOCK, SO_STATUS, &sock_status);
 while(sock_status == SOCK_ESTABLISHED)
 {
	connected = 1;
	osDelay(200);
 }
 printf("Disconnected! ");
 }
 else // Something went wrong
 {
 printf("Failed! \n\r");
 }
 // Close the socket and start a connection again
 close(APP_SOCK);
 printf("Socket closed.\n\r");
 }
 else // Can't open the socket. This may mean something is wrong with W5500 configuration
 {
 printf("Failed to open socket!\n\r");
 }
 // Wait 500 msec before opening
osDelay(500);
}

/* Define Thread 1,  */
void thread_sample(void const *argument)
{
	for(;;)
	{
		
		osSignalWait(0x01, osWaitForever);
		
		if(connected == 1)
		{
		// Get time
		millisec = SysTick_ms();
		
		// Get current velocity
		encoder = Peripheral_Timer_ReadEncoder();
		velocity = Controller_CalculateVelocity(encoder, millisec);
		
		// check status of connection
		getsockopt(APP_SOCK, SO_STATUS, &sock_status);
			
		// If connection not online
		if(sock_status != SOCK_ESTABLISHED)
		{
			// Stop motor
			Peripheral_PWM_ActuateMotor(stop_signal);
		}
		
		// If connection online
		else
		{
		// signal that there is new data to transmit
		osSignalSet(thread_client_ID, 0x01);
		}
		}
	}
}

/* Define Thread 2, */
void thread_actuate(void const *argument)
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		
		// Apply control signal to motor
		Peripheral_PWM_ActuateMotor(control);

	}
}

/* Define Thread 3, */
void thread_client(void const *argument)
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		velocity_boosted = velocity + 20000;
		uvelocity = (uint32_t) velocity_boosted;
		send(APP_SOCK, (uint8_t*)&uvelocity, sizeof(uvelocity));
		returnvalue = recv(APP_SOCK, (uint8_t*)&ucontrol, sizeof(ucontrol));
		control_boosted = (int32_t) ucontrol;
		control = control_boosted - 20000;
		osSignalSet(thread_actuate_ID, 0x01);
	}
}

/* Define callback function for thread wakeup */
void callback(void const *param)
{
	switch( (uint32_t) param)
	{
		case 0:
			osSignalSet(thread_sample_ID, 0x01);
		break;

		case 1:
			osSignalSet(thread_actuate_ID, 0x01);
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
	osTimerId timer_sample = osTimerCreate(osTimer(timer_sample_ID), osTimerPeriodic, (void *)0);	
	
	// initialize CMSIS-RTOS
	osKernelInitialize ();

	// Start timers
	osTimerStart(timer_sample, PERIOD_CTRL);	
	
	// Start threads
	thread_sample_ID = osThreadCreate(osThread(thread_sample), NULL);
	thread_actuate_ID = osThreadCreate(osThread(thread_actuate), NULL);
	thread_client_ID = osThreadCreate(osThread(thread_client), NULL);
	
  // start thread execution 
	osKernelStart ();    
	

	return 0;
}

/* Define what to do in the infinite loop */
void Application_Loop()
{	
	main_ID = osThreadGetId();
	bootup();
}
