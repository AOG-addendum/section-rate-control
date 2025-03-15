

#include "main.hpp"

bool PauseAdjust;
unsigned long CurrentAdjustTime;
float Integral;

int DoPID() {
    int Result = 0;
    // adjusting rate
    if (Sensor.FlowEnabled){
        float ErrorPercent = abs(Sensor.RateError / Sensor.TargetUPM);
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
    return Result;
}

float LastPWM;

int ControlMotor() {
    float Result = 0;
    float ErrorPercent = 0;

    if (Sensor.FlowEnabled && Sensor.TargetUPM > 0) {
        Result = LastPWM;
        ErrorPercent = abs(Sensor.RateError / Sensor.TargetUPM) * 100.0;
        if (ErrorPercent > (float)pidConfig.Deadband) {
            Result += ((float)pidConfig.KP / 255.0) * Sensor.RateError * 5.0;

            if (Result > (float)pidConfig.HighMax) Result = (float)pidConfig.HighMax;
            if (Result < pidConfig.MinPWM) Result = (float)pidConfig.MinPWM;
        }
    }

    LastPWM = Result;
    return (int)Result;
}

