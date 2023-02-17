
#include <Wire.h>
#include "main.hpp"

// rate control with Teensy 4.1
# define InoDescription "RCteensy   02-Jan-2023"
#define MaxReadBuffer 100	// bytes

uint8_t ErrorCount;
bool IOexpanderFound;
bool AutoOn;

unsigned long UDPmillis;

void rateController20Hz ( void* z ) {
  constexpr TickType_t xFrequency = 50;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for( ;; ) {

		Sensor.FlowEnabled = (millis() - Sensor.CommTime < 4000) && Sensor.RateSetting > 0 && Sensor.MasterOn;

		GetUPM();
		AdjustFlow();

		if (AutoOn)
		{
			AutoControl();
		}
		else
		{
			ManualControl();
		}
		if( millis() - UDPmillis > 200 ){
			UDPmillis = millis();
			SendData();
		}
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}


void initRateController() {
	xTaskCreate( rateController20Hz, "rateController", 4096, NULL, 5, NULL );
}

byte ParseModID(byte ID)
{
	// top 4 bits
	return ID >> 4;
}

byte ParseSenID(byte ID)
{
	// bottom 4 bits
	return (ID & 0b00001111);
}

byte BuildModSenID(byte Mod_ID, byte Sen_ID)
{
	return ((Mod_ID << 4) | (Sen_ID & 0b00001111));
}

bool GoodCRC(byte Data[], byte Length)
{
	byte ck = CRC(Data, Length - 1, 0);
	bool Result = (ck == Data[Length - 1]);
	return Result;
}

byte CRC(byte Chk[], byte Length, byte Start)
{
	byte Result = 0;
	int CK = 0;
	for (int i = Start; i < Length; i++)
	{
		CK += Chk[i];
	}
	Result = (byte)CK;
	return Result;
}

void AutoControl()
{
	Sensor.RateError = Sensor.RateSetting - Sensor.UPM;

	if (Sensor.CalOn)
	{
		// calibration mode 
		Sensor.pwmSetting = Sensor.CalPWM;
	}
	else
	{
		// normal mode
		switch (Sensor.ControlType)
		{
		case 2:
		case 3:
			// motor control
			Sensor.pwmSetting = ControlMotor();
			break;

		default:
			// valve control
			Sensor.pwmSetting = DoPID();
			break;
		}
	}
	
}

void ManualControl()
{
	Sensor.RateError = Sensor.RateSetting - Sensor.UPM;
	if (Sensor.CalOn)
	{
		// calibration mode 
		Sensor.pwmSetting = Sensor.CalPWM;
	}
	else
	{
		// normal mode
		if (millis() - Sensor.ManualLast > 1000)
		{
			Sensor.ManualLast = millis();

			// adjust rate
			if (Sensor.RateSetting == 0) Sensor.RateSetting = 1; // to make FlowEnabled

			switch (Sensor.ControlType)
			{
			case 2:
			case 3:
				// motor control
				if (Sensor.ManualAdjust > 0)
				{
					Sensor.pwmSetting *= 1.10;
					if (Sensor.pwmSetting < 1) Sensor.pwmSetting = pidConfig.MinPWM;
					if (Sensor.pwmSetting > 255) Sensor.pwmSetting = 255;
				}
				else if (Sensor.ManualAdjust < 0)
				{
					Sensor.pwmSetting *= 0.90;
					if (Sensor.pwmSetting < pidConfig.MinPWM) Sensor.pwmSetting = 0;
				}
				break;

			default:
				// valve control
				Sensor.pwmSetting = Sensor.ManualAdjust;
				break;
			}
		}
	}
}
