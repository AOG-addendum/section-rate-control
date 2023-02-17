

#include "main.hpp"

bool PauseAdjust;
unsigned long CurrentAdjustTime;
float Integral;

int DoPID() {
    int Result = 0;
    if (!PauseAdjust || pidConfig.AdjustTime == 0) { // AdjustTime==0 disables timed adjustment
        // adjusting rate
        if (Sensor.FlowEnabled){
            float ErrorPercent = abs(Sensor.RateError / Sensor.RateSetting);
            float ErrorBrake = (float)((float)(pidConfig.BrakePoint / 100.0));
            float Max = (float)pidConfig.HighMax;

            if (ErrorPercent > ((float)(pidConfig.Deadband / 100.0))){
                if (ErrorPercent <= ErrorBrake) Max = pidConfig.LowMax;

                Result = (int)((pidConfig.KP * Sensor.RateError) + (Integral * pidConfig.KI / 255.0));
                bool IsPositive = (Result > 0);
                Result = abs(Result);

                if (Result != 0)
                {
                    // limit integral size
                    if ((Integral / Result) < 4) Integral += Sensor.RateError / 3.0;
                }

                if (Result > Max) Result = (int)Max;
                else if (Result < pidConfig.MinPWM) Result = (int)pidConfig.MinPWM;

                if (!IsPositive) Result = -Result;
            }
            else{
                // reset time since no adjustment was made
                CurrentAdjustTime = millis();

                Integral = 0;
            }
        }

        if ((millis() - CurrentAdjustTime) > pidConfig.AdjustTime){
            // switch state
            CurrentAdjustTime = millis();
            PauseAdjust = true;
        }
    }
    else {
        // pausing adjustment, 3 X AdjustTime
        if ((millis() - CurrentAdjustTime) > pidConfig.AdjustTime * 3) {
            // switch state
            CurrentAdjustTime = millis();
            PauseAdjust = false;
        }
    }
    return Result;
}

float LastPWM;

int ControlMotor() {
    float Result = 0;
    float ErrorPercent = 0;

    if (Sensor.FlowEnabled && Sensor.RateSetting > 0) {
        Result = LastPWM;
        ErrorPercent = abs(Sensor.RateError / Sensor.RateSetting) * 100.0;
        if (ErrorPercent > (float)pidConfig.Deadband) {
            Result += ((float)pidConfig.KP / 255.0) * Sensor.RateError * 5.0;

            if (Result > (float)pidConfig.HighMax) Result = (float)pidConfig.HighMax;
            if (Result < pidConfig.MinPWM) Result = (float)pidConfig.MinPWM;
        }
    }

    LastPWM = Result;
    return (int)Result;
}

