#include "controller.h"

//create stored time variable
static uint32_t prev_millisec = 0, dt;

//create controller parameters
static float Kp = 0.25, Ki = 0.0025, integral_0 = 0;

/* Calculate the current velocity in rpm, based on encoder value and time */
int32_t Controller_CalculateVelocity(int16_t encoder, uint32_t millisec)
{
	//declare variables
	int32_t velocity;
	float vel;
	
	//calculate velocity
	dt = millisec - prev_millisec;
	vel = 1000 * ((float)encoder * 60 / 2048) / (float)dt;
	prev_millisec = millisec;
	
	//convert to specified type
	velocity = -((int) vel);
	return velocity;
}

/* Apply a PI-control law to calcuate the control signal for the motor*/
int32_t Controller_PIController(int32_t ref, int32_t current, uint32_t millisec)
{
	//declare variables
	int32_t error, test;
	int16_t DC;
	float output, integral;
	
	//compute controller
	error = ref - current;
	integral = integral_0 + (float) error * (float) dt;
	output = Kp * (float) error + Ki * integral;
	DC = (int16_t)output;
	integral_0 = integral;
	
	//limit control output
	if(DC > 2000)
	{
		DC = 2000;
		Controller_Reset();
	}
	if(DC < -2000)
	{
		DC = -2000;
		Controller_Reset();
	}
	
	return DC;
}

/* Reset internal state variables, such as the integrator */
void Controller_Reset(void)
{
	integral_0 = 0;
	return;
}
