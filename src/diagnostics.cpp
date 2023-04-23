
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
      str = AOGcontrol ? "Automatic (AOG)" : "Manual";
      str += " control";
      str += "\nAOG section state (1...8) ";
      for( int i = 0; i < 8; i++ ){
       str += ( String )( bitRead( sectionsOn, i ));
      }
      str += "\nUpdated ";
      str += ( String )(( millis() - sectionsUpdateMillis ) / 1000 );
      str += " seconds ago";
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
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initDiagnostics() {
  xTaskCreate( diagnosticWorker10Hz, "diagnosticWorker", 3096, NULL, 3, NULL );
}
