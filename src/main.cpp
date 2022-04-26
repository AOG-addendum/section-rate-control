// MIT License
//
// Copyright (c) 2020 Christian Riggenbach
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <string.h>

#include "jsonFunctions.hpp"
#include "main.hpp"

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPUI.h>
#include <AsyncElegantOTA.h>

///////////////////////////////////////////////////////////////////////////
// global data
///////////////////////////////////////////////////////////////////////////
SectionRateConfig sectionRateConfig, sectionRateConfigDefaults;
Initialisation initialisation;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
SemaphoreHandle_t i2cMutex;

const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 );

uint16_t labelLoad;

uint16_t labelStatusOutputType;
uint16_t labelStatusRateControlType;
uint16_t labelStatusMeter;
uint16_t labelStatusSectionsOnWidth;

bool AOG_Control;
///////////////////////////////////////////////////////////////////////////
// external Libraries
///////////////////////////////////////////////////////////////////////////
ESPUIClass ESPUI;
DNSServer dnsServer;

///////////////////////////////////////////////////////////////////////////
// Application
///////////////////////////////////////////////////////////////////////////
void setup( void ) {
  Serial.begin( 115200 );
  WiFi.disconnect( true );

  if( !SPIFFS.begin( true ) ) {
    Serial.println( "SPIFFS Mount Failed" );
    return;
  }
  loadSavedConfig();

  if( sectionRateConfig.gpioAutoManualControl != SectionRateConfig::Gpio::None ) {
    pinMode( ( int ) sectionRateConfig.gpioAutoManualControl, INPUT);
    if( digitalRead( ( int ) sectionRateConfig.gpioAutoManualControl ) == sectionRateConfig.invertAutoManual ) {
      AOG_Control = false;
    } else {
      AOG_Control = true;
    }
  }

  initDOGL_Display();

  Wire.begin( ( int )sectionRateConfig.gpioSDA, ( int )sectionRateConfig.gpioSCL, sectionRateConfig.i2cBusSpeed );

  Serial.updateBaudRate( sectionRateConfig.baudrate );

  if( sectionRateConfig.apModePin != SectionRateConfig::Gpio::None ) {
    pinMode( ( int )sectionRateConfig.apModePin, OUTPUT );
    digitalWrite( ( int )sectionRateConfig.apModePin, LOW );
  }

  initWiFi();
  apIP = WiFi.localIP();

  dnsServer.start( DNS_PORT, "*", apIP );

  Serial.println( "\n\nWiFi parameters:" );
  Serial.print( "Mode: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? "Station" : "Client" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP() );

  i2cMutex = xSemaphoreCreateMutex();

  initESPUI();
  if( sectionRateConfig.enableOTA ) {
    AsyncElegantOTA.begin( ESPUI.server );
  }

  initIdleStats();
  initRateControl();
  initSectionControl();
  initManualSwitches();
}

void loop( void ) {
  dnsServer.processNextRequest();
  vTaskDelay( 100 );
}
