

#include "main.hpp"
#include "jsonFunctions.hpp"

uint8_t manualPWM = 0;

void manualRate100Hz ( void* z ) {
  constexpr TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for( ;; ) {
    if( digitalRead( sectionRateConfig.gpioRateUp ) == LOW ){
      if( manualPWM < 240 ){
        manualPWM += 10;
      } else {
        manualPWM = 255;
      }
      digitalWrite(Sensor.RevPin, LOW);
      delay(1);
      digitalWrite(Sensor.FwdPin, HIGH);
      ledcWrite( 0, manualPWM );
      {
        Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
        String str;
        str.reserve( 30 );
        str = AOGrateControl ? "Automatic (AOG)" : "Manual";
        str += " control";
        str += "\nIncrease flow, ";
        str += ( uint8_t ) manualPWM;
        str += " PWM";
        labelRateValveHandle->value = str;
        labelRateValveHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelRateValveHandle );
      }
    }
    else if( digitalRead( sectionRateConfig.gpioRateDown ) == LOW ){
      if( manualPWM < 240 ){
        manualPWM += 10;
      } else {
        manualPWM = 255;
      }
      digitalWrite(Sensor.FwdPin, LOW);
      delay(1);
      digitalWrite(Sensor.RevPin, HIGH);
      ledcWrite( 0, manualPWM );
      {
        Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
        String str;
        str.reserve( 30 );
        str = AOGrateControl ? "Automatic (AOG)" : "Manual";
        str += " control";
        str += "\nDecrease flow, ";
        str += ( uint8_t ) manualPWM;
        str += " PWM";
        labelRateValveHandle->value = str;
        labelRateValveHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelRateValveHandle );
      }
    }
    else{
      manualPWM = 0;
      digitalWrite(Sensor.FwdPin, LOW);
      digitalWrite(Sensor.RevPin, LOW);
      ledcWrite( 0, manualPWM );
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
