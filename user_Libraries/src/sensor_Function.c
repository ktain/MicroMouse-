#include "global.h"
#include "sensor_Function.h"
#include "adc.h"
#include "delay.h"
#include "pwm.h"
#include "led.h"
#include <stdio.h>
#include "encoder.h"
#include "buzzer.h"

int reflectionRate = 1000;	//which is 1.000 (converted to ingeter)

int32_t volMeter = 0;
int32_t voltage = 0;
int32_t LFSensor = 0;
int32_t RFSensor = 0;
int32_t LDSensor = 0;
int32_t RDSensor = 0;
int32_t LSSensor = 0;
int32_t RSSensor = 0;
int32_t Outz = 0;
int32_t aSpeed = 0;	//angular velocity
int32_t angle = 0; 

/*read IR sensors*/
void readSensor(void)
{
	u32 curt;
	//read DC value		
	
	LFSensor = read_LF_Sensor;	
	RFSensor = read_RF_Sensor;	
	LDSensor = read_LD_Sensor;
	RDSensor = read_RD_Sensor;	
	LSSensor = read_LS_Sensor;
	RSSensor = read_RS_Sensor;
	
	curt = micros();
	
	// Read left and right front sensor
	LEM_F_ON;
	REM_F_ON;
	elapseMicros(60,curt);	// default 60
	LFSensor = read_LF_Sensor - LFSensor;
	RFSensor = read_RF_Sensor - RFSensor;
	LEM_F_OFF;
	REM_F_OFF;
	if(LFSensor < 0)
		LFSensor = 0;
	if(RFSensor < 0)
		RFSensor = 0;
 	elapseMicros(140,curt); // default 140

	
	// Read left and right diagonal sensors
	LEM_D_ON;
	REM_D_ON;
	elapseMicros(340,curt);	// default 340
	LDSensor = read_LD_Sensor - LDSensor;
	RDSensor = read_RD_Sensor - RDSensor;
  LEM_D_OFF;
	REM_D_OFF;
	if(LDSensor < 0)
		LDSensor = 0;
	if(RDSensor < 0)
		RDSensor = 0;
	/*
	elapseMicros(400,curt); // default 140
	
	// Read left and right side sensors
	LEM_S_ON;
	REM_S_ON;
	elapseMicros(150,curt);	// default 340
	LSSensor = read_LS_Sensor - LSSensor;
	RSSensor = read_RS_Sensor - RSSensor;
  LEM_S_OFF;
	REM_S_OFF;
	if(LSSensor < 0)
		LSSensor = 0;
	if(RSSensor < 0)
		RSSensor = 0;
	*/
	
	LFSensor = LFSensor*reflectionRate/1000;
	RFSensor = RFSensor*reflectionRate/1000;
	LDSensor = LDSensor*reflectionRate/1000;
	RDSensor = RDSensor*reflectionRate/1000;
	LSSensor = LSSensor*reflectionRate/1000;
	RSSensor = RSSensor*reflectionRate/1000;
	
	//	10us left in 1ms interrupt
}


void readDiagSensors(void) {
	u32 curt;
	
	LDSensor = read_LD_Sensor;
	RDSensor = read_RD_Sensor;	
	
	curt = micros();
	
	LEM_D_ON;
	REM_D_ON;
	elapseMicros(340,curt);	// default 340
	LDSensor = read_LD_Sensor - LDSensor;
	RDSensor = read_RD_Sensor - RDSensor;
  LEM_D_OFF;
	REM_D_OFF;
	if(LDSensor < 0)
		LDSensor = 0;
	if(RDSensor < 0)
		RDSensor = 0;
	
	LDSensor = LDSensor*reflectionRate/1000;
	RDSensor = RDSensor*reflectionRate/1000;
}

/*read gyro*/
void readGyro(void)
{
	//int curt = micros();								// Use when not in 1ms interrupt
	
	//k=19791(sum for sample in 1 second)    101376287 for 50 seconds with 5000 samples
	int i;
	int sampleNum = 20;
	aSpeed = 0;
	for(i=0;i<sampleNum;i++){
		aSpeed += read_Outz;
	}
	/*
	aSpeed /= sampleNum;
	aSpeed -= read_Vref;
	aSpeed /= 4;
	angle += aSpeed;
	*/
	
	
  aSpeed *= 50000/sampleNum;	// scale by 50000 before dividing by number of samples
	aSpeed -= 93515000;					// 1870.03 scaled by 50000
	aSpeed /= 50000;						// readjust scale
	aSpeed /= 8;							// Scale to degrees * 100, lower = higher degrees
	angle += aSpeed; 
	
	//while( (micros() - curt) < 1000 );	// Use when not in 1ms ISR
}

/*read voltage meter*/
void readVolMeter(void)
{
	volMeter = read_Vol_Meter;//raw value 2611 == 7.00V
	voltage = volMeter/3.105; //actual voltage value 7.00V == 700
	// 10K || 30K
	// 1.75V == 7V == 0x0000087E
}

void lowBatCheck(void)
{
	readVolMeter();
	
	// Low battery threshold of 7.00V
  if(voltage < 700)
	{	
		
		// Stop motors
		setLeftPwm(0);
		setRightPwm(0);
		
		// Turn off all LEDs and emitters
		ALL_LED_OFF;
		
		// Keep buzzer on
		while(1)
		{
			shortBeep(200, 1000); //1kHz tone
		}
	}
}


