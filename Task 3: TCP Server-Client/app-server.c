#include "main.h"

#include "application.h"
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os.h"
#include "socket.h"
#include <stdio.h>



#define PERIOD_CTRL 10
#define PERIOD_REF 4000
#define APP_SOCK 0
#define SERVER_PORT 2103
#define CLIENT_PORT 2104

/* Function Prototypes */
void callback(void const *param);
void thread_reference (void const *argument);
void thread_calculate (void const *argument);
//void thread_server (void const *argument);

/* Thread setup */
osThreadId main_ID, thread_reference_ID, thread_calculate_ID, thread_server_ID;
osThreadDef(thread_reference, osPriorityNormal, 1, 0);
osThreadDef(thread_calculate, osPriorityNormal, 1, 0);
//osThreadDef(thread_server, osPriorityBelowNormal, 1, 0);

/* Timer setup */
osTimerDef(timer_reference_ID, callback);


/* Global variables ----------------------------------------------------------*/
int16_t encoder;
int32_t reference, velocity, control;
uint32_t uvelocity, ucontrol;
int32_t velocity_boosted, control_boosted;
uint32_t millisec;
uint16_t msg;
uint16_t msgret;
uint8_t returnvalue;
uint8_t reception;
uint8_t connected = 0;
uint32_t newcontrol;
uint8_t server_addr[4] = {192, 168, 0, 10};
uint8_t retval;
uint8_t sock_status;

/* Functions -----------------------------------------------------------------*/

/* Define bootup sequence */
void bootup()
{
	printf("Opening socket... ");
 // Open socket
 if((retval = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK)
 {
 printf("Success!\n\r");
 // Put socket in listen mode
 printf("Listening... ");
 if((retval = listen(APP_SOCK)) == SOCK_OK)
 {
 printf("Success!\n\r");
 retval = getsockopt(APP_SOCK, SO_STATUS, &sock_status);
 while (sock_status == SOCK_LISTEN || sock_status == SOCK_ESTABLISHED)
 {
 // If the client has connected, print the message
 if (sock_status == SOCK_ESTABLISHED)
 {
  returnvalue = recv(APP_SOCK, (uint8_t*)&uvelocity, sizeof(uvelocity));
	velocity_boosted = (int32_t) uvelocity;
	velocity = velocity_boosted - 20000;
	osSignalSet(thread_calculate_ID, 0x01);
	osSignalWait(0x01, osWaitForever); 
	control_boosted = control + 20000;
	ucontrol = (uint32_t) control_boosted;
	send(APP_SOCK, (uint8_t*)&ucontrol, sizeof(ucontrol));
	retval = getsockopt(APP_SOCK, SO_STATUS, &sock_status);
 }
 // Otherwise, wait for 100 msec and check again
 else 
 {
 osDelay(100);
 retval = getsockopt(APP_SOCK, SO_STATUS, &sock_status);
 }
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
void thread_reference(void const *argument)
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		
		reference = -reference;
	}
}


/* Define Thread 2,  */
void thread_calculate(void const *argument)
{
	for(;;)
	{
		osSignalWait(0x01, osWaitForever);
		
		// Get time
		millisec = SysTick_ms();
	
		// Calculate control signal
		control = Controller_PIController(reference, velocity, millisec);
		
		// signal that data is ready to send
		osSignalSet(main_ID, 0x01);
	}
}

/* Define Thread 3, */
void thread_server(void const *argument)
{
	for(;;)
	{
		if(connected == 1)
		{
		osDelay(2);
		returnvalue = recv(APP_SOCK, (uint8_t*)&uvelocity, sizeof(uvelocity));
		velocity_boosted = (int32_t) uvelocity;
		velocity = velocity_boosted - 20000;
		osSignalSet(thread_calculate_ID, 0x01);
		osSignalWait(0x01, osWaitForever); 
		control_boosted = control + 20000;
		ucontrol = (uint32_t) control_boosted;
		send(APP_SOCK, (uint8_t*)&ucontrol, sizeof(ucontrol));
		}
		else
		{
			osDelay(10);
		}
	}
}


/* Define callback function for thread wakeup */
void callback(void const *param)
{
	switch( (uint32_t) param)
	{
		case 0:
			osSignalSet(thread_reference_ID, 0x01);
		break;

		case 1:
			osSignalSet(thread_calculate_ID, 0x01);
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
	osTimerId timer_reference = osTimerCreate(osTimer(timer_reference_ID), osTimerPeriodic, (void *)0);	
	
	// initialize CMSIS-RTOS
	osKernelInitialize ();

	// Start timers
	osTimerStart(timer_reference, PERIOD_REF);	
	
	// Start threads
	thread_reference_ID = osThreadCreate(osThread(thread_reference), NULL);
	thread_calculate_ID = osThreadCreate(osThread(thread_calculate), NULL);
	//thread_server_ID = 		osThreadCreate(osThread(thread_server), NULL);
	
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