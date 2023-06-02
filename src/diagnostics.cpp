
#include <stdio.h>
#include <string.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

void diagnosticWorker10Hz( void* z ) {
  vTaskDelay( 2000 );
  constexpr TickType_t xFrequency = 100;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for( ;; ) {

    {
      Control* labelSectionsStatusHandle = ESPUI.getControl( labelSectionStatus );
      String str;
      str.reserve( 30 );
      str = AOGsectionControl ? "Automatic (AOG)" : "Manual";
      str += " control";
      str += "\nAOG section state (1...8) ";
      for( int i = 0; i < 8; i++ ){
       str += ( String )( bitRead( sectionsOn, i ));
      }
      str += "\nUpdated ";
      unsigned long time = millis() - sectionsUpdateMillis;
      if( time > 1000 ){
        str += ( String )( time / 1000 );
        str += " seconds ago";
      } else {
        str += ( String )( time );
        str += " milliseconds ago";
      }
      labelSectionsStatusHandle->value = str;
      labelSectionsStatusHandle->color = ControlColor::Emerald;
      ESPUI.updateControlAsync( labelSectionsStatusHandle );
    }
    {
      Control* labelRateMeterStatusHandle = ESPUI.getControl( labelRateMeter );
      String str;
      str.reserve( 30 );
      str = ( String )totalPulseCount;
      str += " pulses, ";
      str += ( String )( Duration );
      str += " milliseconds duration";
      str += "\nUpdated ";
      unsigned long time = millis() - PulseTime;
      if( time > 1000 ){
        str += ( String )( time / 1000 );
        str += " seconds ago";
      } else {
        str += ( String )( time );
        str += " milliseconds ago";
      }
      labelRateMeterStatusHandle->value = str;
      labelRateMeterStatusHandle->color = ControlColor::Emerald;
      ESPUI.updateControlAsync( labelRateMeterStatusHandle );
    }
    {
      Control* labelRateValveHandle = ESPUI.getControl( labelRateValve );
      String str;
      str.reserve( 30 );
      if( AOGrateControl == true ){
        str = "Automatic (AOG) control\n";
        if( Sensor.pwmSetting > 0 ){
          str += "Increase flow, ";
          str += ( uint8_t ) Sensor.pwmSetting;
          str += " PWM\n";
        }
        else if( Sensor.pwmSetting < 0 ){
          str += "Decrease flow, ";
          str += ( uint8_t ) Sensor.pwmSetting;
          str += " PWM\n";
        } else {
          str += "Maintaining flow\n";
        }
        str += "Updated ";
        unsigned long time = millis() - Sensor.CommTime;
        if( time > 1000 ){
            str += ( String )( time / 1000 );
            str += " seconds ago";
        } else {
            str += ( String )( time );
            str += " milliseconds ago";
        }
      } else {
        str = "Manual control\n";
        if( digitalRead( sectionRateConfig.gpioRateUp ) == LOW ){
          str += "Increase flow, ";
          str += ( uint8_t ) manualPWM;
          str += " PWM";
        }
        else if( digitalRead( sectionRateConfig.gpioRateDown ) == LOW ){
          str += "Decrease flow, ";
          str += ( uint8_t ) manualPWM;
          str += " PWM";
        }
        else {
          str += "Maintaining flow";
        }
      }
      labelRateValveHandle->value = str;
      labelRateValveHandle->color = ControlColor::Emerald;
      ESPUI.updateControlAsync( labelRateValveHandle );
    }
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initDiagnostics() {
  xTaskCreate( diagnosticWorker10Hz, "diagnosticWorker", 3096, NULL, 3, NULL );
}
