/*
* KobukiNavigationStatechart.c
*
*/

#include "kobukiNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>


// Program States
typedef enum{
	INITIAL = 0,						// Initial state
	PAUSE_WAIT_BUTTON_RELEASE,			// Paused; pause button pressed down, wait until released before detecting next press
	UNPAUSE_WAIT_BUTTON_PRESS,			// Paused; wait for pause button to be pressed
	UNPAUSE_WAIT_BUTTON_RELEASE,		// Paused; pause button pressed down, wait until released before returning to previous state
	DRIVE,								// Drive straight
	TURN_R,								// Turn right
	TURN_L,								// Turn left
	REVERSE								// Reverse

} robotState_t;

#define DEG_PER_RAD			(180.0 / M_PI)		// degrees per radian
#define RAD_PER_DEG			(M_PI / 180.0)		// radians per degree

void KobukiNavigationStatechart(
	const int16_t 				maxWheelSpeed,
	const int32_t 				netDistance,
	const int32_t 				netAngle,
	const KobukiSensors_t		sensors,
	const accelerometer_t		accelAxes,
	int16_t * const 			pRightWheelSpeed,
	int16_t * const 			pLeftWheelSpeed,
	const bool					isSimulator
	){

	// local state
	static robotState_t 		state = INITIAL;				// current program state
	static robotState_t			unpausedState = DRIVE;			// state history for pause region
	static int32_t				distanceAtManeuverStart = 0;	// distance robot had travelled when a maneuver begins, in mm
	static int32_t				angleAtManeuverStart = 0;		// angle through which the robot had turned when a maneuver begins, in deg

	// outputs
	int16_t						leftWheelSpeed = 0;				// speed of the left wheel, in mm/s
	int16_t						rightWheelSpeed = 0;			// speed of the right wheel, in mm/s

	// variables
	bool c = sensors.bumps_wheelDrops.bumpCenter;
	bool l = sensors.bumps_wheelDrops.bumpLeft;
	bool r = sensors.bumps_wheelDrops.bumpRight;
	robotState_t prevState = DRIVE;

	//*****************************************************
	// state data - process inputs                        *
	//*****************************************************



	if (state == INITIAL
		|| state == PAUSE_WAIT_BUTTON_RELEASE
		|| state == UNPAUSE_WAIT_BUTTON_PRESS
		|| state == UNPAUSE_WAIT_BUTTON_RELEASE
		|| sensors.buttons.B0				// pause button
		){
		switch (state){
		case INITIAL:
			// set state data that may change between simulation and real-world
			if (isSimulator){
			}
			else{
			}
			state = UNPAUSE_WAIT_BUTTON_PRESS; // place into pause state
			break;
		case PAUSE_WAIT_BUTTON_RELEASE:
			// remain in this state until released before detecting next press
			if (!sensors.buttons.B0){
				state = UNPAUSE_WAIT_BUTTON_PRESS;
			}
			break;
		case UNPAUSE_WAIT_BUTTON_RELEASE:
			// user pressed 'pause' button to return to previous state
			if (!sensors.buttons.B0){
				state = unpausedState;
			}
			break;
		case UNPAUSE_WAIT_BUTTON_PRESS:
			// remain in this state until user presses 'pause' button
			if (sensors.buttons.B0){
				state = UNPAUSE_WAIT_BUTTON_RELEASE;
			}
			break;
		default:
			// must be in run region, and pause button has been pressed
			unpausedState = state;
			state = PAUSE_WAIT_BUTTON_RELEASE;
			break;
		}
	}
	//*************************************
	// state transition - run region      *
	//*************************************
	/*else if (state == DRIVE && abs(netDistance - distanceAtManeuverStart) <= 50){
		angleAtManeuverStart = netAngle;
		distanceAtManeuverStart = netDistance;
		if (prevState == TURN_L) {
			state = TURN_R;
		}
		else if (prevState == TURN_R) {
			state = TURN_L;
		}
		prevState = DRIVE;
	}*/
	else if ((state == TURN_L || state == TURN_R) && abs(netAngle - angleAtManeuverStart) >= 90){
		angleAtManeuverStart = netAngle;
		distanceAtManeuverStart = netDistance;
		state = DRIVE;
		prevState = DRIVE;
	}
	else if (state == DRIVE){
		if (abs(netDistance - distanceAtManeuverStart) <= 50) {
			angleAtManeuverStart = netAngle;
			distanceAtManeuverStart = netDistance;
			if (prevState == TURN_L) {
				state = TURN_R;
			}
			else if (prevState == TURN_R) {
				state = TURN_L;
			}
			prevState = DRIVE;
		}

		if (c) {
			if (l) {
				if (r) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = TURN_R;
					prevState = TURN_R;
				}				
			}
			else if (r) {
				if (l) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = TURN_L;
					prevState = TURN_L;
				}	
			}
			else {
				state = REVERSE;
				prevState = REVERSE;
			}
		}
		else if (l) {
			if (c) {
				if (r) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = TURN_R;
					prevState = TURN_R;
				}
			}
			else if (r) {
				if (c) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = REVERSE;
					prevState = REVERSE;
				}
			}
			else {
				state = TURN_R;
				prevState = TURN_R;
			}
		}
		else if (r) {
			if (c) {
				if (l) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = TURN_L;
					prevState = TURN_L;
				}	
			}
			else if (l) {
				if (c) {
					state = REVERSE;
					prevState = REVERSE;
				}
				else {
					state = REVERSE;
					prevState = REVERSE;
				}
			}
			else {
				state = TURN_L;
				prevState = TURN_L;
			}
		}
	}
	// else, no transitions are taken

	//*****************
	//* state actions *
	//*****************
	switch (state){
	case INITIAL:
	case PAUSE_WAIT_BUTTON_RELEASE:
	case UNPAUSE_WAIT_BUTTON_PRESS:
	case UNPAUSE_WAIT_BUTTON_RELEASE:
		// in pause mode, robot should be stopped
		leftWheelSpeed = rightWheelSpeed = 0;
		break;

	case DRIVE:
		// full speed ahead!
		leftWheelSpeed = rightWheelSpeed = 100;
		break;

	case TURN_R:
		leftWheelSpeed = 100;
		rightWheelSpeed = -leftWheelSpeed;
		break;

	case TURN_L:
		rightWheelSpeed = 100;
		leftWheelSpeed = -rightWheelSpeed;
		break;
	
	case REVERSE:
		leftWheelSpeed = rightWheelSpeed = -100;
		break;

	default:
		// Unknown state
		leftWheelSpeed = rightWheelSpeed = 0;
		break;
	}


	*pLeftWheelSpeed = leftWheelSpeed;
	*pRightWheelSpeed = rightWheelSpeed;
}
