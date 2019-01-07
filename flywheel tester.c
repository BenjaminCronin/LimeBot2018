#pragma config(Motor,  port2,           flywheel1,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           flywheel2,     tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           flywheel3,     tmotorVex393_MC29, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

bool flipped = false;

task main()
{
	while (true)
	{
		while(vexRT[Btn6U])
		{
			flipped = !flipped;
		}
		if (flipped)
		{
			motor[flywheel3] = -127;
			for(float i = 0; i <= 63; i += 0.005)
			{
				motor[flywheel1] = motor[flywheel2] = i;
			}
			while(true)
			{
				if(vexRT[Btn6U])
				{
					break;
				}
			}
			motor[flywheel3] = 0;
			for(float i = 63; i >= 0; i -= 0.05)
			{
				motor[flywheel1] = motor[flywheel2] = 0;
			}
		}
	}
}
