/* 
 * ****************************************************************************
 * RP6 ROBOT SYSTEM 
 * ****************************************************************************
 * Description:
 *
 *
 */

/*****************************************************************************/
// Includes:
#include "globalIncludes.h"


/*****************************************************************************/
// Cruise Behaviour:

#define CRUISE_SPEED_FWD    50// 100 Default speed when no obstacles are detected!
uint8_t speed = CRUISE_SPEED_FWD;

#define MOVE_FORWARDS 1
struct behaviour_command_t cruise = {CRUISE_SPEED_FWD, CRUISE_SPEED_FWD, FWD, false, false, 0, MOVE_FORWARDS};

/**
 * We don't have anything to do here - this behaviour has only
 * a constant value for moving forwards - s. above!
 * Of course you can change this and add some random or timed movements 
 * in here...
 */
void behaviour_cruise(void)
{
	if(cruise.state == IDLE)
		return;
		
	if( !isStopwatch5Running())
	{
		writeString_P("Start Cruise\n"); 
		speed = CRUISE_SPEED_FWD;
		setStopwatch5(0);
		startStopwatch5();
	}

	if(getStopwatch5() > 1000)
	{
		setStopwatch5(500);
		startStopwatch5();
		
		
		if(speed < 150)
		{
			speed += 1;
		}
		
		if(speed < 100)
		{
			speed += 1;
		}
		
		
		if(speed < 80)
		{
			speed += 1;
		}
		
		cruise.speed_left = speed;
		cruise.speed_right = speed;
		writeString_P("Speed: "); 
		writeInteger(speed, DEC);
		writeString_P("\n"); 
	}
}
