

#include "main.hpp"
#include "jsonFunctions.hpp"

void manualRate100Hz ( void* z ) {
  constexpr TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t pwmSetting = 0;
  for( ;; ) {
    if( digitalRead( sectionRateConfig.gpioRateUp ) == LOW ){
      if( pwmSetting < 240 ){
        pwmSetting += 10;
      } else {
        pwmSetting = 255;
      }
      digitalWrite(Sensor.RevPin, LOW);
      delay(1);
      digitalWrite(Sensor.FwdPin, HIGH);
      ledcWrite( 0, pwmSetting );
      {
        Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
        String str;
        str.reserve( 30 );
        str = AOGrateControl ? "Automatic (AOG)" : "Manual";
        str += " control";
        str += "\nIncrease flow, ";
        str += ( uint8_t ) pwmSetting;
        str += " PWM";
        labelRateValveHandle->value = str;
        labelRateValveHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelRateValveHandle );
      }
    }
    else if( digitalRead( sectionRateConfig.gpioRateDown ) == LOW ){
      if( pwmSetting < 240 ){
        pwmSetting += 10;
      } else {
        pwmSetting = 255;
      }
      digitalWrite(Sensor.FwdPin, LOW);
      delay(1);
      digitalWrite(Sensor.RevPin, HIGH);
      ledcWrite( 0, pwmSetting );
      {
        Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
        String str;
        str.reserve( 30 );
        str = AOGrateControl ? "Automatic (AOG)" : "Manual";
        str += " control";
        str += "\nDecrease flow, ";
        str += ( uint8_t ) pwmSetting;
        str += " PWM";
        labelRateValveHandle->value = str;
        labelRateValveHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelRateValveHandle );
      }
    }
    else{
      pwmSetting = 0;
      digitalWrite(Sensor.FwdPin, LOW);
      digitalWrite(Sensor.RevPin, LOW);
      ledcWrite( 0, pwmSetting );
      {
        Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
        String str;
        str.reserve( 30 );
        str = AOGrateControl ? "Automatic (AOG)" : "Manual";
        str += " control";
        str += "\nMaintaining flow";
        labelRateValveHandle->value = str;
        labelRateValveHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelRateValveHandle );
      }
    }
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initManualRate(){
	xTaskCreate( manualRate100Hz, "manualRate", 4096, NULL, 5, NULL );
}
