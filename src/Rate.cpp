

#include "main.hpp"

bool previousState;
const int SampleSize = 24;
uint32_t Samples[SampleSize];
const uint32_t PulseMin = 250;
volatile uint32_t PulseMax = 50000;
volatile unsigned long PulseCount;
volatile unsigned long Duration;
volatile unsigned long PulseLast;
volatile unsigned long PulseTime;
volatile unsigned long totalPulseCount;
volatile int SamplesCount;
volatile uint32_t SamplesTotal;
volatile uint16_t SamplesIndex;
uint32_t LastPulse;
double PulseAvg;

unsigned long TimedCounts;
uint32_t RateInterval;
uint32_t RateTimeLast;
uint32_t PWMTimeLast;

unsigned long CurrentCount;
uint32_t CurrentTotal;
uint32_t CurrentDuration;

void GetUPM_Old(){
	if (Sensor.ControlType == 3){
		// use weight
		Sensor.UPM = Sensor.MeterCal * (double)Sensor.pwmSetting;
	}
	else{
		// use flow meter
		GetUPMflow();
	}
}

void GetUPM(){
	if (PulseCount && Sensor.MeterCal > 0){
		LastPulse = millis();

		noInterrupts();
		Sensor.TotalPulses += PulseCount;
		PulseCount = 0;
		if (SamplesCount > SampleSize) SamplesCount = SampleSize;
		CurrentCount = SamplesCount;
		CurrentTotal = SamplesTotal;
		interrupts();

		PulseAvg = ((double)CurrentTotal / CurrentCount) * 0.8 + PulseAvg * 0.2;
		Sensor.UPM = (double)(60000000.0 / PulseAvg) / Sensor.MeterCal;
		PulseMax = PulseAvg * 1.5;
	}

	// check for no flow
	if (millis() - LastPulse > 4000){
		Sensor.UPM = 0;
		PulseMax = 500000;
		SamplesCount = 0;
		SamplesIndex = 0;
		SamplesTotal = 0;
		memset(Samples, 0, sizeof(Samples));
	}
}

void IRAM_ATTR ISR0(){
	bool state = digitalRead( Sensor.FlowPin );
	if( previousState != state ){
		previousState = state;
		if( state == HIGH ){
			PulseTime = micros();
			Duration = PulseTime - PulseLast;
			if ( Duration > PulseMin ){
				PulseLast = PulseTime;
				if ( Duration < PulseMax ){
					PulseCount++;
					SamplesTotal -= Samples[SamplesIndex];
					Samples[SamplesIndex] = Duration;
					SamplesTotal += Samples[SamplesIndex];
					SamplesIndex = (SamplesIndex + 1) % SampleSize;
					SamplesCount++;
				}
			}
		}
	}
}

void initFlowMeterInterrupts(){
	pinMode( Sensor.FlowPin, INPUT);
	attachInterrupt( digitalPinToInterrupt( Sensor.FlowPin ), ISR0, CHANGE);
}

