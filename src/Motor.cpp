

#include "main.hpp"

void AdjustFlow(){
    switch (Sensor.ControlType) {
    case 1:
        // fast close valve, used for flow control and on/off
        if (Sensor.FlowEnabled) {
            if (Sensor.pwmSetting >= 0) {
                //increase
                if (Sensor.pwmSetting > 250)	Sensor.pwmSetting = 255;

                digitalWrite(Sensor.DirPin, MDL.FlowOnDirection);
                analogWrite(Sensor.PWMPin, Sensor.pwmSetting);
            }
            else {
                //decrease
                if (Sensor.pwmSetting < -250) Sensor.pwmSetting = -255;

                digitalWrite(Sensor.DirPin, !MDL.FlowOnDirection);
                analogWrite(Sensor.PWMPin, -Sensor.pwmSetting);	// offsets the negative pwm value
            }
        }
        else {
            // stop flow
            digitalWrite(Sensor.DirPin, !MDL.FlowOnDirection);
            analogWrite(Sensor.PWMPin, 255);
        }
        break;

    case 2:
    case 3:
        // motor control
        if (Sensor.FlowEnabled) {
            if (Sensor.pwmSetting >= 0) {
                //increase
                digitalWrite(Sensor.DirPin, MDL.FlowOnDirection);
                analogWrite(Sensor.PWMPin, Sensor.pwmSetting);
            }
            else {
                //decrease
                digitalWrite(Sensor.DirPin, !MDL.FlowOnDirection);
                analogWrite(Sensor.PWMPin, -Sensor.pwmSetting);	// offsets the negative pwm value
            }
        }
        else {
            // stop motor
            analogWrite (Sensor.PWMPin, 0);
        }
        break;

    default:
        // standard valve, flow control only
        if (Sensor.pwmSetting > 0) {
            //increase
            digitalWrite(Sensor.RevPin, LOW);
            delay(1);
            digitalWrite(Sensor.FwdPin, HIGH);
            ledcWrite( 0, Sensor.pwmSetting );
            {
                Control* labelRateMotorHandle = ESPUI.getControl( labelRateMotor );
                String str;
                str.reserve( 30 );
                str = AOGcontrol ? "Automatic (AOG)" : "Manual";
                str += " control";
                str += "\nIncrease flow, ";
                str += ( uint8_t ) Sensor.pwmSetting;
                str += " PWM";
                labelRateMotorHandle->value = str;
                labelRateMotorHandle->color = ControlColor::Emerald;
                ESPUI.updateControlAsync( labelRateMotorHandle );
            }
        }
        else if (Sensor.pwmSetting < 0) {
            //decrease
            digitalWrite(Sensor.FwdPin, LOW);
            delay(1);
            digitalWrite(Sensor.RevPin, HIGH);
            ledcWrite( 0, -Sensor.pwmSetting );	// offsets the negative pwm value
            {
                Control* labelRateMotorHandle = ESPUI.getControl( labelRateMotor );
                String str;
                str.reserve( 30 );
                str = AOGcontrol ? "Automatic (AOG)" : "Manual";
                str += " control";
                str += "\nDecrease flow, ";
                str += ( uint8_t ) -Sensor.pwmSetting;
                str += " PWM";
                labelRateMotorHandle->value = str;
                labelRateMotorHandle->color = ControlColor::Emerald;
                ESPUI.updateControlAsync( labelRateMotorHandle );
            }
        } else {
            digitalWrite(Sensor.FwdPin, LOW);
            delay(1);
            digitalWrite(Sensor.RevPin, LOW);
            ledcWrite( 0, 0 );	// offsets the negative pwm value            
            {
                Control* labelRateMotorHandle = ESPUI.getControl( labelRateMotor );
                String str;
                str.reserve( 30 );
                str = AOGcontrol ? "Automatic (AOG)" : "Manual";
                str += " control";
                str += "\nMaintaining flow";
                labelRateMotorHandle->value = str;
                labelRateMotorHandle->color = ControlColor::Emerald;
                ESPUI.updateControlAsync( labelRateMotorHandle );
            }
        }
        break;
    }
}
