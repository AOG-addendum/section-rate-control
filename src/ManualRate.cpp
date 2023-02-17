

#include "main.hpp"
#include "jsonFunctions.hpp"

void manualRate100Hz ( void* z ) {
  constexpr TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t pwmSetting = 0;
  for( ;; ) {
    if( digitalRead( sectionRateConfig.gpioRateUp ) == HIGH ){
      pwmSetting += 1;
      if( pwmSetting > 255 ){
        pwmSetting = 255;
      }
      digitalWrite(Sensor.RevPin, LOW);
      delay(1);
      digitalWrite(Sensor.FwdPin, HIGH);
      ledcWrite( 0, pwmSetting );
    }
    else if( digitalRead( sectionRateConfig.gpioRateDown ) == HIGH ){
      pwmSetting += 1;
      if( pwmSetting > 255 ){
        pwmSetting = 255;
      }
      digitalWrite(Sensor.FwdPin, LOW);
      delay(1);
      digitalWrite(Sensor.RevPin, HIGH);
      ledcWrite( 0, Sensor.pwmSetting );
    }
    else{
      pwmSetting = 0;
      digitalWrite(Sensor.FwdPin, LOW);
      digitalWrite(Sensor.RevPin, LOW);
      ledcWrite( 0, pwmSetting );
    }
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initManualRate(){
	xTaskCreate( manualRate100Hz, "manualRate", 4096, NULL, 5, NULL );
}