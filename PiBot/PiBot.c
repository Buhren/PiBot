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

#include "State_Escape.c"
#include "State_Avoid.c"
#include "State_Cruise.c"
#include "State_RemoteControl.c"

/*****************************************************************************/
// Behaviour command type:

/*****************************************************************************/
// Behaviour control:

/**
 * This function processes the movement commands that the behaviours generate. 
 * Depending on the values in the behaviour_command_t struct, it sets motor
 * speed, moves a given distance or rotates.
 */
void moveCommand(struct behaviour_command_t * cmd)
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
	behaviour_escape();
	behaviour_avoid();
	behaviour_remoteControl();
	//behaviour_cruise();


    // Execute the commands depending on priority levels:
	if(escape.state != IDLE) 				// Highest priority - 4
		moveCommand(&escape);
	else if(avoid.state != IDLE) 			// Priority - 3
		moveCommand(&avoid);
	else if(remoteControl.state != IDLE) 	// Priority - 2
		moveCommand(&remoteControl); 
	//else if(cruise.state != IDLE) 		// Priority - 1
	//	moveCommand(&cruise); 
	else                     				// Lowest priority - 0
		moveCommand(&STOP);  				// Default command - do nothing! 
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
	
	writeString_P("Initialisierungen beginnen\n");
	
	
	// Dies ist relevant falls der RP6 als I2C-Slave verwendet werden soll. Bspw. wenn er ferngesteuert werden soll:
	I2CTWI_initSlave (RP6BASE_I2C_SLAVE_ADR ); 	
	
	// Set Bumpers state changed event handler:
	BUMPERS_setStateChangedHandler(bumpersStateChangedNew);
	
	// Set ACS state changed event handler:
	ACS_setStateChangedHandler(acsStateChangedNew);
	
	powerON(); // Turn on Encoders, Current sensing, ACS and Power LED.
	
	// Activates the Anti-Collision-System. The range can vary by using setACSPwrLow(), setACSPwrMed() or setACSPwrHigh()
	setACSPwrMed();
	
	writeString_P("PiBot gestartet\n");
	
	// Main loop
	while(true) 
	{
		// Hier wird das aktuelle Verhalten des Roboters evaluiert
		behaviourController();
		
		// Hier wird die zyklische Auswertung des ACS, Bumper und ADC durchgeführt. Zudem werden hier die Motoren angesteuert
		task_RP6System(); 
	}
	return 0;
}
