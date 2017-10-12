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

#include "RP6RobotBaseLib.h" 	
#include "RP6Base_I2CSlave.c"

/*****************************************************************************/
// Behaviour command type:

#define IDLE  0


// The behaviour command data type:
typedef struct {
	uint8_t  speed_left;  // left speed (is used for rotation and 
						  // move distance commands - if these commands are 
						  // active, speed_right is ignored!)
	uint8_t  speed_right; // right speed
	unsigned dir:2;       // direction (FWD, BWD, LEFT, RIGHT)
	unsigned move:1;      // move flag
	unsigned rotate:1;    // rotate flag
	uint16_t move_value;  // move value is used for distance and angle values
	uint8_t  state;       // state of the behaviour
} behaviour_command_t;

behaviour_command_t STOP = {0, 0, FWD, false, false, 0, IDLE};

/*****************************************************************************/
// Cruise Behaviour:

#define CRUISE_SPEED_FWD    50// 100 Default speed when no obstacles are detected!
uint8_t speed = CRUISE_SPEED_FWD;

#define MOVE_FORWARDS 1
behaviour_command_t cruise = {CRUISE_SPEED_FWD, CRUISE_SPEED_FWD, FWD, false, false, 0, MOVE_FORWARDS};

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

/*****************************************************************************/
// Remote Control Behaviour:

#define REMOTE_CONTROL_SPEED    50


#define MOVE_STOP		1
#define MOVE_FORWARD	2
#define MOVE_LEFT		3
#define MOVE_BACKWARD	4
#define MOVE_RIGHT		5

behaviour_command_t remoteControl = {0, 0, FWD, false, false, 0, IDLE};

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

/*****************************************************************************/
// Escape Behaviour:

#define ESCAPE_SPEED_BWD    40 // 100
#define ESCAPE_SPEED_ROTATE 30  // 60

#define ESCAPE_FRONT		1
#define ESCAPE_FRONT_WAIT 	2
#define ESCAPE_LEFT  		3
#define ESCAPE_LEFT_WAIT	4
#define ESCAPE_RIGHT	    5
#define ESCAPE_RIGHT_WAIT 	6
#define ESCAPE_WAIT_END		7
behaviour_command_t escape = {0, 0, FWD, false, false, 0, IDLE}; 

/**
 * This is the Escape behaviour for the Bumpers.
 */
void behaviour_escape(void)
{
	static uint8_t bump_count = 0;
	
	switch(escape.state)
	{
		case IDLE: 
		break;
		case ESCAPE_FRONT:
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir = BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 220;
			else
				escape.move_value = 160;
			escape.state = ESCAPE_FRONT_WAIT;
			bump_count+=2;
		break;
		case ESCAPE_FRONT_WAIT:			
			if(!escape.move) // Wait for movement to be completed
			{	
				escape.speed_left = ESCAPE_SPEED_ROTATE;
				if(bump_count > 3)
				{
					escape.move_value = 100;
					escape.dir = RIGHT;
					bump_count = 0;
				}
				else 
				{
					escape.dir = LEFT;
					escape.move_value = 70;
				}
				escape.rotate = true;
				escape.state = ESCAPE_WAIT_END;
			}
		break;
		case ESCAPE_LEFT:
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir 	= BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 190;
			else
				escape.move_value = 150;
			escape.state = ESCAPE_LEFT_WAIT;
			bump_count++;
		break;
		case ESCAPE_LEFT_WAIT:
			if(!escape.move) // Wait for movement to be completed
			{
				escape.speed_left = ESCAPE_SPEED_ROTATE;
				escape.dir = RIGHT;
				escape.rotate = true;
				if(bump_count > 3)
				{
					escape.move_value = 110;
					bump_count = 0;
				}
				else
					escape.move_value = 80;
				escape.state = ESCAPE_WAIT_END;
			}
		break;
		case ESCAPE_RIGHT:	
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir 	= BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 190;
			else
				escape.move_value = 150;
			escape.state = ESCAPE_RIGHT_WAIT;
			bump_count++;
		break;
		case ESCAPE_RIGHT_WAIT:
			if(!escape.move) // Wait for movement to be completed
			{ 
				escape.speed_left = ESCAPE_SPEED_ROTATE;		
				escape.dir = LEFT;
				escape.rotate = true;
				if(bump_count > 3)
				{
					escape.move_value = 110;
					bump_count = 0;
				}
				else
					escape.move_value = 80;
				escape.state = ESCAPE_WAIT_END;
			}
		break;
		case ESCAPE_WAIT_END:
			if(!(escape.move || escape.rotate)) // Wait for movement/rotation to be completed
				escape.state = IDLE;
		break;
	}
}

/**
 * Bumpers Event handler
 */
void bumpersStateChangedNew(void)
{
	if(bumper_left && bumper_right) // Both Bumpers were hit
	{
		escape.state = ESCAPE_FRONT;
	}
	else if(bumper_left)  			// Left Bumper was hit
	{
		if(escape.state != ESCAPE_FRONT_WAIT) 
			escape.state = ESCAPE_LEFT;
	}
	else if(bumper_right) 			// Right Bumper was hit
	{
		if(escape.state != ESCAPE_FRONT_WAIT)
			escape.state = ESCAPE_RIGHT;
	}
	stopStopwatch5(); 	// Reinitialisierung der Anfahrt-GeschwindigkeitsRampe
}

/*****************************************************************************/
// The new Avoid Behaviour:

// Some speed values for different movements:
#define AVOID_SPEED_L_ARC_LEFT  30
#define AVOID_SPEED_L_ARC_RIGHT 40 // 90
#define AVOID_SPEED_R_ARC_LEFT  40 // 90
#define AVOID_SPEED_R_ARC_RIGHT 30
#define AVOID_SPEED_ROTATE 	30     // 60

// States for the Avoid FSM:
#define AVOID_OBSTACLE_RIGHT 		1
#define AVOID_OBSTACLE_LEFT 		2
#define AVOID_OBSTACLE_MIDDLE	    3
#define AVOID_OBSTACLE_MIDDLE_WAIT 	4
#define AVOID_END 					5
behaviour_command_t avoid = {0, 0, FWD, false, false, 0, IDLE};

/**
 * The new avoid behaviour. It uses the two ACS channels to avoid
 * collisions with obstacles. It drives arcs or rotates depending
 * on the sensor states and also behaves different after some
 * detecting cycles to avoid lock up situations. 
 */
void behaviour_avoid(void)
{
	static uint8_t last_obstacle = LEFT;
	static uint8_t obstacle_counter = 0;
	switch(avoid.state)
	{
		case IDLE: 
		// This is different to the escape Behaviour where
		// we used the Event Handler to detect sensor changes...
		// Here we do this within the states!
			if(obstacle_right && obstacle_left) // left AND right sensor have detected something...
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			else if(obstacle_left)  // Left "sensor" has detected something
				avoid.state = AVOID_OBSTACLE_LEFT;
			else if(obstacle_right) // Right "sensor" has detected something
				avoid.state = AVOID_OBSTACLE_RIGHT;
		break;
		case AVOID_OBSTACLE_MIDDLE:
			avoid.dir = last_obstacle;
			avoid.speed_left = AVOID_SPEED_ROTATE;
			avoid.speed_right = AVOID_SPEED_ROTATE;
			if(!(obstacle_left || obstacle_right))
			{
				if(obstacle_counter > 3)
				{
					obstacle_counter = 0;
					setStopwatch4(0);
				}
				else
					setStopwatch4(400);
				startStopwatch4();
				avoid.state = AVOID_END;
			}
		break;
		case AVOID_OBSTACLE_RIGHT:
			avoid.dir = FWD;
			avoid.speed_left = AVOID_SPEED_L_ARC_LEFT;
			avoid.speed_right = AVOID_SPEED_L_ARC_RIGHT;
			if(obstacle_right && obstacle_left)
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			if(!obstacle_right)
			{
				setStopwatch4(500);
				startStopwatch4();
				avoid.state = AVOID_END;
			}
			last_obstacle = RIGHT;
			obstacle_counter++;
		break;
		case AVOID_OBSTACLE_LEFT:
			avoid.dir = FWD;
			avoid.speed_left = AVOID_SPEED_R_ARC_LEFT;
			avoid.speed_right = AVOID_SPEED_R_ARC_RIGHT;
			if(obstacle_right && obstacle_left)
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			if(!obstacle_left)
			{
				setStopwatch4(500); 
				startStopwatch4();
				avoid.state = AVOID_END;
			}
			last_obstacle = LEFT;
			obstacle_counter++;
		break;
		case AVOID_END:
			if(getStopwatch4() > 1000) // We used timing based movement here!
			{
				stopStopwatch4();
				setStopwatch4(0);
				avoid.state = IDLE;
			}
		break;
	}
}

/**
 * ACS Event Handler - ONLY used for LED display! 
 * This does not affect the behaviour at all! 
 * The sensor checks are completely done in the Avoid behaviour
 * statemachine.
 */
void acsStateChangedNew(void)
{
	if(obstacle_left && obstacle_right)
		statusLEDs.byte = 0b100100;
	else
		statusLEDs.byte = 0b000000;
	statusLEDs.LED5 = obstacle_left;
	statusLEDs.LED4 = (!obstacle_left);
	statusLEDs.LED2 = obstacle_right;
	statusLEDs.LED1 = (!obstacle_right);
	updateStatusLEDs();
	stopStopwatch5(); 	// Reinitialisierung der Anfahrt-GeschwindigkeitsRampe
}

/*****************************************************************************/
// Behaviour control:

/**
 * This function processes the movement commands that the behaviours generate. 
 * Depending on the values in the behaviour_command_t struct, it sets motor
 * speed, moves a given distance or rotates.
 */
void moveCommand(behaviour_command_t * cmd)
{
	if(cmd->move_value > 0)  // move or rotate?
	{
		if(cmd->rotate)
			rotate(cmd->speed_left, cmd->dir, cmd->move_value, false); 
		else if(cmd->move)
			move(cmd->speed_left, cmd->dir, DIST_MM(cmd->move_value), false); 
		cmd->move_value = 0; // clear move value - the move commands are only
		                     // given once and then runs in background.
	}
	else if(!(cmd->move || cmd->rotate)) // just move at speed? 
	{
		changeDirection(cmd->dir);
		moveAtSpeed(cmd->speed_left,cmd->speed_right);
	}
	else if(isMovementComplete()) // movement complete? --> clear flags!
	{
		cmd->rotate = false;
		cmd->move = false;
	}
}

/**
 * The behaviourController task controls the subsumption architechture. 
 * It implements the priority levels of the different behaviours. 
 */
void behaviourController(void)
{
    // Call all the behaviour tasks:
	//behaviour_cruise();
	behaviour_remoteControl();
	behaviour_avoid();
	behaviour_escape();


    // Execute the commands depending on priority levels:
	if(escape.state != IDLE) // Highest priority - 3
		moveCommand(&escape);
	else if(avoid.state != IDLE) // Priority - 2
		moveCommand(&avoid);
	else if(remoteControl.state != IDLE) // Priority - 1
		moveCommand(&remoteControl); 
	//else if(cruise.state != IDLE) // Priority - 1
	//	moveCommand(&cruise); 
	else                     // Lowest priority - 0
		moveCommand(&STOP);  // Default command - do nothing! 
							 // In the current implementation this never 
							 // happens.
}

/*****************************************************************************/
// Main:

int main(void)
{
	initRobotBase(); 
	setLEDs(0b111111);
	mSleep(2500);
	setLEDs(0b100100);
	
	I2CTWI_initSlave (RP6BASE_I2C_SLAVE_ADR );
	
	writeString_P("Initialisierungen beginnen\n");

	// Set Bumpers state changed event handler:
	BUMPERS_setStateChangedHandler(bumpersStateChangedNew);
	
	// Set ACS state changed event handler:
	ACS_setStateChangedHandler(acsStateChangedNew);
	
	powerON(); // Turn on Encoders, Current sensing, ACS and Power LED.
	setACSPwrMed(); 
	
	writeString_P("PiBot gestartet\n");
	// Main loop
	while(true) 
	{		
		behaviourController();
		task_RP6System();
	}
	return 0;
}
