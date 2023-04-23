

#include "main.hpp"

volatile bool state;
volatile bool previousState;
volatile unsigned long Duration;
volatile unsigned long PulseCount;
volatile unsigned long PulseTime;
volatile unsigned long totalPulseCount;
uint32_t LastPulse;

unsigned long TimedCounts;
uint32_t RateInterval;
uint32_t RateTimeLast;
uint32_t PWMTimeLast;

unsigned long CurrentCount;
uint32_t CurrentDuration;

unsigned long PPM;		// pulse per minute * 100
unsigned long Osum;
unsigned long Omax;
unsigned long Omin;
byte Ocount;
float Oave;
unsigned long Omax2;
unsigned long Omin2;

void IRAM_ATTR ISR0(){
	state = digitalRead( ( uint8_t )Sensor.FlowPin );
	if (previousState != state){
		previousState = state;
		Duration = millis() - PulseTime;
		PulseTime = millis();
		PulseCount++;
		totalPulseCount++;
	}
}

void GetUPM(){
	if (Sensor.ControlType == 3){
		// use weight
		Sensor.UPM = Sensor.MeterCal * (double)Sensor.pwmSetting;
	}
	else{
		// use flow meter
		GetUPMflow();
	}
}

void GetUPMflow(){
	if (PulseCount){
		noInterrupts();
		CurrentCount = PulseCount;
		PulseCount = 0;
		CurrentDuration = Duration;
		interrupts();

		if (Sensor.UseMultiPulses){
			// low ms/pulse, use pulses over time
			TimedCounts += CurrentCount;
			RateInterval = millis() - RateTimeLast;
			if (RateInterval > 500){
				RateTimeLast = millis();
				PPM = (6000000 * TimedCounts) / RateInterval;	// 100 X actual
				TimedCounts = 0;
			}
		}
		else{
			// high ms/pulse, use time for one pulse
			if (CurrentDuration == 0){
				PPM = 0;
			}
			else{
				PPM = 6000000 / CurrentDuration;	// 100 X actual
			}
		}

		LastPulse = millis();
		Sensor.TotalPulses += CurrentCount;
	}

	if (millis() - LastPulse > 4000)	PPM = 0;	// check for no flow

	// double olympic average
	Osum += PPM;
	if (Omax < PPM){
		Omax2 = Omax;
		Omax = PPM;
	}
	else if (Omax2 < PPM) Omax2 = PPM;

	if (Omin > PPM){
		Omin2 = Omin;
		Omin = PPM;
	}
	else if (Omin2 > PPM) Omin2 = PPM;

	Ocount++;
	if (Ocount > 9){
		Osum -= Omax;
		Osum -= Omin;
		Osum -= Omax2;
		Osum -= Omin2;
		Oave = (float)Osum / 600.0;	// divide by 6 samples and divide by 100 for decimal place
		Osum = 0;
		Omax = 0;
		Omin = 5000000;
		Omax2 = 0;
		Omin2 = 5000000;
		Ocount = 0;
	}

	// units per minute
	if (Sensor.MeterCal > 0){
		Sensor.UPM = Oave / Sensor.MeterCal;
	}
	else{
		Sensor.UPM = 0;
	}
}


