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
// Remote Control Behaviour:

#define REMOTE_CONTROL_SPEED    50


#define MOVE_STOP		1
#define MOVE_FORWARD	2
#define MOVE_LEFT		3
#define MOVE_BACKWARD	4
#define MOVE_RIGHT		5

struct behaviour_command_t remoteControl = {0, 0, FWD, false, false, 0, IDLE};

/**
 * We don't have anything to do here - this behaviour has only
 * a constant value for moving forwards - s. above!
 * Of course you can change this and add some random or timed movements 
 * in here...
 */
void behaviour_remoteControl(void)
{
	if(getCommand()) 	// Wurde ein Kommando empfangen?
	{
		writeString_P("Kommando empfangen\n"); 
		remoteControl.state = cmd;
	}
	else	// Ist kein Kommando angekommen?
	{
		return;
	}


	if(remoteControl.state == IDLE)	// Gibt es nichts zu tun?
		return;
	
	// Standard-Geschwindigkeit wird gesetzt
	remoteControl.speed_left  = REMOTE_CONTROL_SPEED;
	remoteControl.speed_right = REMOTE_CONTROL_SPEED;
			
	switch( cmd )
	{
		case MOVE_STOP:
			writeString_P("Remote Control: Stop\n"); 
			remoteControl.speed_left = 0;
			remoteControl.speed_right = 0;
			break;
			
		case MOVE_FORWARD:
			writeString_P("Remote Control: Forwards\n"); 
			remoteControl.dir = FWD;
			break;
			
		case MOVE_BACKWARD:
			writeString_P("Remote Control: Backwards\n"); 
			remoteControl.dir = BWD;
			break;
			
		case MOVE_LEFT:
			writeString_P("Remote Control: Left\n"); 
			remoteControl.dir = LEFT;
			break;
			
		case MOVE_RIGHT:
			writeString_P("Remote Control: Right\n");
			remoteControl.dir = RIGHT;
			break;
			
		case IDLE:
			writeString_P("Remote Control: Idle\n");
			remoteControl.speed_left = 0;
			remoteControl.speed_right = 0;
			break;
			
		default:
			writeString_P("Remote Control: Invalid command: ");
			writeInteger(cmd, DEC);
			writeString_P("\n");
			remoteControl.speed_left = 0;
			remoteControl.speed_right = 0;
			break;
	}
	writeString_P("\n");
}

