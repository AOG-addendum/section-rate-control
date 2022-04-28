
#include <dog_7565R.h>
#include <font_16x32nums.h>
#include <font_6x8.h>
#include <font_8x16.h>
#include <font_8x8.h>

#include "main.hpp"
#include "jsonFunctions.hpp"
#include "display.hpp"
#include "AOG_bmp.hpp"


dog_7565R DOG;

char DISPLAY_TEXT [menuitems][17] {
  "gallons per acre",
  "gallons applied",
  "miles per hour",
  "acres applied",
  "gallons in tank"
};
double DISPLAY_VARIABLES [menuitems] {
  0.00,
  0.00,
  0.00,
  0.00,
  0.00
};
int DISPLAY_INDEX [menuitems] {
  0,
  1,
  2,
  3,
  4
};

bool reset = false;

void updateDisplayVariables( void ){
  if( digitalRead(21) == LOW ){
    return;
  }
  char var[25] = {};
  int index = DISPLAY_INDEX[0];
  dtostrf(DISPLAY_VARIABLES[index], 5, 2, var);
  int offset = (128 - (String(DISPLAY_VARIABLES[index]).length() * 8)) / 2; //center the number, each digit is 8 points wide
  DOG.string(0, 2, font_8x16, "                "); // clear artifacts
  DOG.string(offset, 2, font_8x16, var);

  index = DISPLAY_INDEX[1];
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 5, font_6x8, var);

  index = DISPLAY_INDEX[2];
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 6, font_6x8, var);

  index = DISPLAY_INDEX[3];
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 7, font_6x8, var);
}

void updateDisplay( void ){
  DOG.clear();
  char var[25] = {};
  int index = DISPLAY_INDEX[0];
  int offset = (128 - (String(DISPLAY_TEXT[index]).length() * 8)) / 2; //center the message, each char is 8 points wide
  DOG.string(offset, 0, font_8x16, DISPLAY_TEXT[index]);
  dtostrf(DISPLAY_VARIABLES[index], 5, 2, var);
  offset = (128 - (String(DISPLAY_VARIABLES[index]).length() * 8)) / 2; //center the number, each digit is 8 points wide
  DOG.string(offset, 2, font_8x16, var);
  DOG.rectangle(10, 4, 117, 4, 0x20);

  index = DISPLAY_INDEX[1];
  DOG.string(0, 5, font_6x8, DISPLAY_TEXT[index]);
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 5, font_6x8, var);

  index = DISPLAY_INDEX[2];
  DOG.string(0, 6, font_6x8, DISPLAY_TEXT[index]);
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 6, font_6x8, var);

  index = DISPLAY_INDEX[3];
  DOG.string(0, 7, font_6x8, DISPLAY_TEXT[index]);
  dtostrf(DISPLAY_VARIABLES[index], 5, 0, var);
  DOG.string(99, 7, font_6x8, var);
}

void checkManualUpSwitch (void) {
  static bool debounceState = HIGH;
  static bool lastState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static uint8_t rate = 100;
  bool state = digitalRead(19);
  if (state != lastState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > 50) {
    debounceState = state;
  }
  if (state == LOW and debounceState == LOW) {
    if( rate > 200 ){
      rate = 255;
    } else {
      rate += 1;
    }
    ledcWrite( 0, rate );
    ledcWrite( 1, 0 );
  } else {
    rate = 100;
  }
  lastState = state;
}

void checkAutoUpSwitch (void) {
  static bool debounceState = HIGH;
  static bool lastState = HIGH;
  static unsigned long lastDebounceTime = 0;
  bool state = digitalRead(19);
  if (state != lastState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > 50) {
    if (state == LOW and debounceState == HIGH) {
      for (uint8_t i = 0; i < menuitems; i++) {
        DISPLAY_INDEX[i] += 1;
        if (DISPLAY_INDEX[i] >= menuitems) {
          DISPLAY_INDEX[i] = 0;
        }
      }
      runtimeData.currentDisplayIndex = DISPLAY_INDEX[0];
      updateDisplay();
    }
    debounceState = state;
  }
  lastState = state;
}

void checkManualDownSwitch (void) {
  static bool debounceState = HIGH;
  static bool lastState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static uint8_t rate = 100;
  bool state = digitalRead(21);
  if (state != lastState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > 50) {
    debounceState = state;
  }
  if (state == LOW && debounceState == LOW) {
    if( rate > 128 ){
      rate = 255;
    } else {
      rate += 1;
    }
    ledcWrite( 0, 0 );
    ledcWrite( 1, rate );
  } else {
    rate = 100;
  }
  lastState = state;
}

void checkAutoDownSwitch (void) {
  static unsigned long startMillis = 0;
  static int previousSeconds = 0;
  static bool lastState = HIGH;
  bool state = digitalRead(21);
  if (state != lastState and state == HIGH) {
    updateDisplay();
    reset = false;
  }
  lastState = state;
  if (state == LOW and reset == false) {
    int seconds = 3 - ((millis() - startMillis) / 1000);
    if (previousSeconds != seconds) {
      previousSeconds = seconds;
      DOG.string(0, 2, font_6x8, "                      ");
      DOG.string(0, 3, font_6x8, "reset in ");
      char secondsString [1];
      itoa(seconds, secondsString, 10);
      DOG.string(54, 3, font_6x8, secondsString);
      DOG.string(60, 3, font_6x8, " seconds");
    }
    if (seconds == 0) {
      if( DISPLAY_INDEX[0] == 4 ){
        runtimeData.tankFluidRemaining = sectionRateConfig.tankSize;
        DISPLAY_VARIABLES[4] = runtimeData.tankFluidRemaining;
      } else if ( DISPLAY_INDEX[0] == 3 ) {
        runtimeData.totalLandUnitsApplied = 0;
        DISPLAY_VARIABLES[3] = runtimeData.totalLandUnitsApplied;
      } else if ( DISPLAY_INDEX[0] == 1 ) {
        runtimeData.totalFluidUnitsApplied = 0;
        DISPLAY_VARIABLES[1] = runtimeData.totalFluidUnitsApplied;
      }
      reset = true;
      updateDisplay();
    }
    return;  // do not reset start_millis
  }
  startMillis = millis();
  previousSeconds = 4; // force the display to update when hitting the switch rapidly
}

void displayManualWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  DOG.initialize(2, 23, 18, 5, 4, DOGL128); //SS,SI,SCK,A0,RESET,EA DOGL128-6 (=128x64 dots)
  DOG.view(VIEW_TOP);
  DOG.picture(32, 0, Image_AOG_bmp);
  vTaskDelay( 1500 / portTICK_PERIOD_MS );
  DOG.string(8, 4, font_8x16, "Manual control");
  vTaskDelay( 1000 / portTICK_PERIOD_MS );
  updateDisplay();

  for( ;; ) {
    updateDisplayVariables();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void switchManualWorker10Hz( void* z ) {
  constexpr TickType_t xFrequency = 100;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for( ;; ) {
    checkManualUpSwitch();
    checkManualDownSwitch();
    if( digitalRead(19) == HIGH && digitalRead(21) == HIGH ){
      ledcWrite( 0, 0 );
      ledcWrite( 1, 0 );
    }
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void displayAutoWorker10Hz( void* z ) {
  constexpr TickType_t xFrequency = 100;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  DOG.initialize(2, 23, 18, 5, 4, DOGL128); //SS,SI,SCK,A0,RESET,EA DOGL128-6 (=128x64 dots)
  DOG.view(VIEW_TOP);
  DOG.picture(32, 0, Image_AOG_bmp);
  vTaskDelay( 1500 / portTICK_PERIOD_MS );
  DOG.string(20, 4, font_8x16, "AOG control");
  vTaskDelay( 1000 / portTICK_PERIOD_MS );
  updateDisplay();

  for( ;; ) {
    checkAutoUpSwitch();
    checkAutoDownSwitch();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initDOGL_Display( void ){

  if( AOG_Control == false ) {
    xTaskCreate( displayManualWorker1Hz, "displayManualWorker", 3096, NULL, 3, NULL );
  } else {
    xTaskCreate( displayAutoWorker10Hz, "displayAutoWorker", 3096, NULL, 3, NULL );
  }

  loadSavedRuntimeData();
  for( uint8_t i = 0; i < menuitems; i ++ ){
    uint8_t index = i + runtimeData.currentDisplayIndex;
    if( index > ( menuitems - 1 )){
      index -= menuitems - 1;
    }
    DISPLAY_INDEX[i] = index;
  }
}

void initManualSwitches( void ){
  if( AOG_Control == false ) {
    xTaskCreate( switchManualWorker10Hz, "switchManualWorker", 3096, NULL, 3, NULL );
  }
}
