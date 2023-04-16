
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
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initDiagnostics() {
  xTaskCreate( diagnosticWorker10Hz, "diagnosticWorker", 3096, NULL, 3, NULL );
}
