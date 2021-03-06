#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  bump,           sensorTouch)
#pragma config(Sensor, dgtl5,  autonGood,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl6,  confirmAuton,   sensorDigitalIn)
#pragma config(Sensor, dgtl7,  autonBad,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl8,  dig0,           sensorDigitalIn)
#pragma config(Sensor, dgtl10, dig1,           sensorDigitalIn)
#pragma config(Sensor, dgtl12, dig2,           sensorDigitalIn)
#pragma config(Sensor, I2C_1,  FRDriveIME,     sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  FLDriveIME,     sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  FStrafeIME,     sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_4,  BStrafeIME,     sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_5,  SlingshotIME,   sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           strafeBack,    tmotorVex393_HBridge, openLoop, reversed, encoderPort, I2C_4)
#pragma config(Motor,  port2,           driveFR,       tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port3,           driveFL,       tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port4,           slingWIME,     tmotorVex393_MC29, openLoop, encoderPort, I2C_5)
#pragma config(Motor,  port5,            ,             tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,            ,             tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           slingWOIME,    tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           driveBR,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           driveBL,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          strafeFront,   tmotorVex393_HBridge, openLoop, encoderPort, I2C_3)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "Vex_Competition_Includes.c"
#include "utils.c"

#define MULTIPLIER -2
#define MAX 10
#define THRESHOLD 5

const float TICKS_PER_DEGREE_SPEED = (392.0/360.0);
const float TICKS_PER_DEGREE_TORQUE = (627.2/360.0);
const float ERROR_MULTIPLIER = 2;
const int ERROR_ACCEPTENCE = 25;

task autonSelection();
task blinkRedLEDHalfSec();
task blinkGreenLEDHalfSec();

float targetPoint;

int auton = 0;

//int drive = 0;
//int bumpSwitchState = 0;
//int numberOfDriveModes = 4;

void ResetDriveI2CEncoders()
{
	nMotorEncoder[driveFR] = nMotorEncoder[driveFL] = nMotorEncoder[strafeBack] = nMotorEncoder[strafeFront] = 0;
}
void ResetSlingshotI2CEncoders()
{
	nMotorEncoder[slingWIME] = 0;
}

//PID Funcs
void InitPID()
{
	targetPoint = 0;
	ResetSlingshotI2CEncoders();
}
void setTargetPoint(bool go, int direction, int currentPoint)
{
	if(go)
	{
		if(direction > 0)
		{
			targetPoint++;
		}
		else if(direction < 0)
		{
			targetPoint--;
		}
		if(targetPoint > currentPoint + MAX)
		{
			targetPoint = currentPoint + MAX;
		}
		else if(targetPoint < currentPoint - MAX)
		{
			targetPoint = currentPoint - MAX;
		}
	}
}
int PID(int currentPoint)
{
	int pwr;
	if(targetPoint > currentPoint - THRESHOLD
		|| targetPoint < currentPoint + THRESHOLD)
	{
		pwr = MULTIPLIER * (targetPoint - currentPoint);
	}
	if(pwr > 127)
	{
		pwr = 127;
	}
	else if(pwr < -127)
	{
		pwr = -127;
	}
	return pwr;
}

// Drive functions
void LDrive(int pwr)
{
	motor[driveBL] = pwr;
	motor[driveFL] = pwr;
}
void RDrive(int pwr)
{
	motor[driveBR] = pwr;
	motor[driveFR] = pwr;
}
void Strafe(int pwr)
{
	motor[strafeBack] = pwr;
	motor[strafeFront] = pwr;
}
void Slingshot(int pwr)
{
	motor[slingWIME] = motor[slingWOIME] = pwr;
}

// Auton functions
/*
Neil_6121C 8 Dec 2017 Pennsylvania 6121C said:
If you are using a 393 motor, the number of ticks per revolution [counted by the I2C] will be:

627.2 for torque gearing
392 for high speed gearing
261.333 for turbo gearing
*/
/*
Current configuration:
All drive motors: high speed
Strafe: high speed
Slingshot: Torque

Wheels: 32.5cm circumfrence (all)
*/

//Counted out of 360 degrees
void moveDegrees(int motorDegrees)
{
	//resets the encoders in preperation for population
	ResetDriveI2CEncoders();
	int ticksToMove = round(motorDegrees * TICKS_PER_DEGREE_SPEED);
	//turns motors on, returns if luser wants to rotate 0 degrees
	if(ticksToMove > 0)
	{
		LDrive(127);
		RDrive(127);
	}
	else if(ticksToMove < 0)
	{
		LDrive(-127);
		RDrive(-127);
	}
	else
	{
		return;
	}
	//waits for .25 seconds to populate the IMEs
	wait1Msec(250);
	//Don't bother turning off the motors, we're just going to reset the speeds later in this function
	//LDrive(0);
	//RDrive(0);
	//left makes positive, right makes negitive
	if(ticksToMove > 0)
	{
		while(nMotorEncoder[driveFL] <= ticksToMove && nMotorEncoder[driveFR] <= ticksToMove)
		{
			int temp = nMotorEncoder[driveFL] - nMotorEncoder[driveFR];
			float difference;
			if((sign(temp) == 1) && (temp >= ERROR_ACCEPTENCE))
			{
				//abs for contingincy where it starts out backwards, for some reason
				difference = fabs(nMotorEncoder[driveFR] / (nMotorEncoder[driveFL] * ERROR_MULTIPLIER));
				RDrive(127);
				LDrive(127 * difference);
			}
			else if((sign(temp) == -1) && (temp <= (-1 * ERROR_ACCEPTENCE)))
			{
				difference = nMotorEncoder[driveFL] / (nMotorEncoder[driveFR] * ERROR_MULTIPLIER);
				LDrive(127);
				RDrive(127 * difference);
			}
			else//if(sign(temp) == 0)
			{
				LDrive(127);
				RDrive(127);
			}
		}
	}
	else
	{
		while(nMotorEncoder[driveFL] >= ticksToMove && nMotorEncoder[driveFR] >= ticksToMove)
		{
			int temp = nMotorEncoder[driveFL] - nMotorEncoder[driveFR];
			float difference;
			if((sign(temp) == 1) && (temp >= ERROR_ACCEPTENCE))
			{
				difference = fabs(nMotorEncoder[driveFR] / (nMotorEncoder[driveFL] * ERROR_MULTIPLIER));
				RDrive(-127);
				LDrive(-127 * difference);
			}
			else if((sign(temp) == -1) && (temp <= (-1 * ERROR_ACCEPTENCE)))
			{
				difference = nMotorEncoder[driveFL] / (nMotorEncoder[driveFR] * ERROR_MULTIPLIER);
				LDrive(-127);
				RDrive((-127) * difference);
			}
			else//if(sign(temp) == 0)
			{
				LDrive(-127);
				RDrive(-127);
			}
		}
	}
	LDrive(-1 * sign(ticksToMove) * 127);
	RDrive(-1 * sign(ticksToMove) * 127);
	wait1Msec(100);
	LDrive(0);
	RDrive(0);
}
void moveTurns(float turns)
{
	moveDegrees(round(turns * 360));
}
//turn right is positive
//up to user to determine how many degrees of _motor rotation_ it will take to get 360 degrees of _robot rotation_
//user aka me
void rotDegrees(int motorDegrees)
{
	int ticksToMove = motorDegrees * TICKS_PER_DEGREE_SPEED;
	LDrive(sign(motorDegrees) * 127);
	RDrive(-sign(motorDegrees) * 127);
	while(nMotorEncoder[driveFL] <= ticksToMove && nMotorEncoder[driveFR] <= ticksToMove)
	{

	}
	LDrive(0);
	RDrive(0);
}
//strafe right is positive
void strafeDegrees(int motorDegrees)
{
	int ticksToMove = motorDegrees * TICKS_PER_DEGREE_SPEED;
	Strafe(sign(motorDegrees) * 127);
	while(nMotorEncoder[strafeFront] <= ticksToMove && nMotorEncoder[strafeBack] <= ticksToMove)
	{

	}
	Strafe(0);
}
void shootSlingshot()
{
	//assumes that the gear starts in "prime" position: gear teeth positioned just above engaging
	//maybe can create something to spin it and see resistance?
	ResetSlingshotI2CEncoders();
	while(nMotorEncoder[slingWIME] <= (360 * TICKS_PER_DEGREE_TORQUE))
	{
		Slingshot(127);
	}
	Slingshot(0);
}

// Drive modes
void tank(int left, int right, int strafe)
{
	LDrive(left);
	RDrive(right);
	motor[strafeBack] = (strafe - (left - right));
	motor[strafeFront] = (strafe + (left - right));
}
void arcade(int mov, int rot, int strafe)
{
	LDrive(mov + rot);
	RDrive(mov - rot);
	motor[strafeBack] = (strafe - rot);
	motor[strafeFront] = (strafe + rot);
}

void AutonRedNearFlag()
{
	moveTurns(-1);//move back into position to shoot flag
	shootSlingshot();//shoot high (middle or top) flag
	rotDegrees(90);//turn to the platform
	moveTurns(4);//however many turns it takes to get up onto the platform
}
void AutonBlueNearFlag()
{
	//https://docs.google.com/drawings/d/17SoShb_IrFyIc0RGJsnIWTWYxXtZrZAFC1ZPVMn4pkw
	moveTurns(-1);//move back into position to shoot flag
	shootSlingshot();//shoot high (middle or top) flag
	rotDegrees(-90);//turn to the platform
	moveTurns(4);//however many turns it takes to get up onto the platform
}
void AutonRedNearPost()
{
	moveTurns(1);
	shootSlingshot();
	rotDegrees(90);
	moveTurns(4);
}
void AutonBlueNearPost()
{
	moveTurns(1);
	shootSlingshot();
	rotDegrees(-90);
	moveTurns(4);
}

bool autonHasBeenSelected = false;
int prevLoopPinStatus;
// VEX Functions/Tasks
void pre_auton()
{
	startTask(autonSelection);
}
task autonSelection()
{
	//wait for the user to put in the select auton jumper clip and blink red LED
	startTask(blinkRedLEDHalfSec);
	while(SensorValue(confirmAuton) == 1)
	{

	}
	prevLoopPinStatus = 0;
	stopTask(blinkRedLEDHalfSec);
	SensorValue(autonBad) = 0;
	startTask(blinkGreenLEDHalfSec);
	while(true)
	{
		//CHANGEME if/f more autons are added (# of autons minus one)
		if(SensorValue(confirmAuton) == 1 && prevLoopPinStatus == 0)
		{
			if(auton < 3)
			{
				stopTask(blinkGreenLEDHalfSec);
				SensorValue(autonGood) = 1;
				SensorValue(autonBad) = 0;
				continue;
			}
			else
			{
				stopTask(blinkGreenLEDHalfSec);
				SensorValue(autonBad) = 1;
				SensorValue(autonGood) = 0;
				continue;
			}
		}
		else if(SensorValue(confirmAuton) == 0 && prevLoopPinStatus == 1)
		{
			SensorValue(autonBad) = 0;
			startTask(blinkGreenLEDHalfSec);
		}
		else if(SensorValue(confirmAuton) == 1 && prevLoopPinStatus == 1)
		{
			continue;
		}
		//if no jumper pin in digital port 12
		if(SensorValue(dig2) == 1)
		{
			//if no jumper in in digital port 10
			if(SensorValue(dig1) == 1)
			{
				//if no jumper pin in digital port 8
				if(SensorValue(dig0) == 1)
				{
					auton = 0;
				}
				//else if jumper pin in digital port 8
				else
				{
					auton = 1;
				}
			}
			//else if jumper pin in digital port 10
			else
			{
				//if no jumper pin in digital port 8
				if(SensorValue(dig0) == 1)
				{
					auton = 2;
				}
				//if jumper pin in digital port 8
				else
				{
					auton = 3;
				}
			}
		}
		//else if jumper pin in digital port 12
		else
		{
			//if no jumper in in digital port 10
			if(SensorValue(dig1) == 1)
			{
				//if no jumper pin in digital port 8
				if(SensorValue(dig0) == 1)
				{
					auton = 4;
				}
				//else if jumper in in digital port 8
				else
				{
					auton = 5;
				}
			}
			//else if jumper pin in digital port 10
			else
			{
				//if no jumper pin in digital port 8
				if(SensorValue(dig0) == 1)
				{
					auton = 6;
				}
				//if jumper pin in digital port 8
				else
				{
					auton = 7;
				}
			}
		}
		prevLoopPinStatus = SensorValue(confirmAuton);
	}
}
task blinkGreenLEDHalfSec()
{
	while(true)
	{
		SensorValue(autonGood) = 1;
		wait1Msec(500);
		SensorValue(autonGood) = 0;
		wait1Msec(500);
	}
}
task blinkRedLEDHalfSec()
{
	while(true)
	{
		SensorValue(autonBad) = 1;
		wait1Msec(500);
		SensorValue(autonBad) = 0;
		wait1Msec(500);
	}
}
task autonomous()
{
	stopTask(autonSelection);
	//defaults to RedNearFlag
	switch(auton)
	{
		case 0:
			AutonRedNearFlag();
			break;
		case 1:
			AutonBlueNearFlag();
			break;
		case 2:
			AutonRedNearPost();
			break;
		case 3:
			AutonBlueNearPost();
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			AutonRedNearFlag();
			break;
	}
}
task usercontrol()
{
	InitPID();
	while(true)
	{
		arcade(vexRT[Ch3], vexRT[Ch1], vexRT[Ch4]);
		if(vexRT[Btn6D])
		{
			setTargetPoint(true, 1, nMotorEncoder[slingWIME]);
		}
		else if(vexRT[Btn6U])
		{
			setTargetPoint(true, -1, nMotorEncoder[slingWIME]);
		}
		motor[slingWIME] = PID(nMotorEncoder[slingWIME]);
	}
}
