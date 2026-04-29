

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

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 ); //IP address for access point
IPAddress ipDestination( 192, 168, 5, 1 ); //IP address to send UDP data to

///////////////////////////////////////////////////////////////////////////
// external Libraries
///////////////////////////////////////////////////////////////////////////
DNSServer dnsServer;

unsigned long WifiSwitchesTimer;
bool WifiSwitchesEnabled;
byte WifiSwitches[6];

bool AOGsectionControl = false;
bool AOGrateControl = false;

// Relays
byte RelayLo;	// sections 0-7
byte RelayHi;	// sections 8-15
byte PowerRelayLo;
byte PowerRelayHi;

bool ESPconnected;
byte ESPdebug1;
int8_t WifiRSSI;
uint32_t WifiTime;
uint32_t WifiLastTime;

int ControlMotor( int );
int DoPID( int );
void AutoControl();

ModuleConfig MDL;
SensorConfig Sensor;
PIDConfig pidConfig, pidConfigDefaults;
AnalogConfig AINs;

///////////////////////////////////////////////////////////////////////////
// helper functions
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Application
///////////////////////////////////////////////////////////////////////////

void setup( void ) {

  xTaskCreate( initDOGL_Display, "initDOGL_Display", 3096, NULL, 3, NULL );

  Serial.begin( 115200 );

  WiFi.disconnect( true );

  if( !SPIFFS.begin( true ) ) {
    Serial.println( "SPIFFS Mount Failed" );
    return;
  }

  loadSavedConfig();
  loadPIDConfig();

  if( sectionRateConfig.pcbVersion == 0 ){ // Rev C
    Sensor.FlowPin = 39;
    Sensor.FwdPin = 17;
    Sensor.RevPin = 16;
    Sensor.PWMPin = 15;
    sectionRateConfig.gpioManualAutoSelection = 36;
  } else if( sectionRateConfig.pcbVersion == 1 ){ // Rev D
    Sensor.FlowPin = 36;
    Sensor.FwdPin = 27;
    Sensor.RevPin = 26;
    Sensor.PWMPin = 25;
    sectionRateConfig.gpioManualAutoSelection = 39;
  } else if( sectionRateConfig.pcbVersion == 2 ){ // Rev E
    Sensor.FlowPin = 34;
    Sensor.FwdPin = 27;
    Sensor.RevPin = 26;
    Sensor.PWMPin = 25;
    sectionRateConfig.gpioManualAutoSelection = 39;
  }

  pinMode( sectionRateConfig.gpioManualAutoSelection, INPUT );
  if( digitalRead( sectionRateConfig.gpioManualAutoSelection ) == LOW ){
    Serial.println("Automatic section control");
    AOGsectionControl = true;
    if( sectionRateConfig.rateControlAlwaysManual == true ){
      Serial.println("Manual rate control");
      AOGrateControl = false;
    } else {
      Serial.println("Automatic rate control");
      AOGrateControl = true;
    }
  } else {
    Serial.println("Manual section control");
    Serial.println("Manual rate control");
    AOGsectionControl = false;
    AOGrateControl = false;
  }

	Wire.begin();

  pinMode(Sensor.RevPin, OUTPUT);
  pinMode(Sensor.FwdPin, OUTPUT);
  pinMode(Sensor.PWMPin, OUTPUT);

  initFlowMeterInterrupts();

  ledcSetup( 0, 1000, 8 );
  ledcAttachPin( Sensor.PWMPin, 0 );
  ledcWrite( 0, 0 );

  Serial.updateBaudRate( sectionRateConfig.baudrate );
  Serial.println( "Welcome to ESP32 Section Rate Control.\nTo configure, please open the WebUI." );

  pinMode( sectionRateConfig.gpioWifiLed, OUTPUT );

  initWiFi();
  apIP = WiFi.localIP();

  dnsServer.start( DNS_PORT, "*", apIP );

  Serial.println( "\n\nWiFi parameters:" );
  Serial.print( "Mode: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? "Station" : "Client" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP() );

  initESPUI();

  if( sectionRateConfig.enableOTA ) {
    AsyncElegantOTA.begin( ESPUI.WebServer() );
  }

  if ( udpSendFrom.listen( sectionRateConfig.rcPortSendFrom ))
  {
    Serial.print( "UDP writing to IP: " );
    Serial.println( ipDestination );
    Serial.print( "UDP writing to port: " );
    Serial.println( sectionRateConfig.rcPortSendTo );
    Serial.print( "UDP writing from port: " );
    Serial.println( sectionRateConfig.rcPortSendFrom );
  }

  if( WiFi.status() == WL_CONNECTED ) { // digitalWrite doesn't work in Wifi callbacks
    digitalWrite( sectionRateConfig.gpioWifiLed, HIGH );
  }

  pinMode( sectionRateConfig.gpioRateUp, INPUT );
  pinMode( sectionRateConfig.gpioRateDown, INPUT );
  pinMode( sectionRateConfig.gpioManualAutoSelection, INPUT );

  Wire.beginTransmission( 0x20 );
  Wire.write( 0x00 ); // IODIRA register
  Wire.write( 0x00 ); // set entire PORT A to output
  Wire.endTransmission();

  if( AOGsectionControl == true ){
    initAutoSectionUDP();
    if( AOGrateControl == false ){
      initManualRate();
    } else {
      initAutoRateController();
      initAutoRateControlUDP();
    }
  } else {
    Wire.beginTransmission( 0x20 );
    Wire.write( 0x12 ); // address port A
    Wire.write( 0x00 );  // all sections off
    Wire.endTransmission();
    initManualRate();
    initManualSection();
  }

  runDOGL_DisplayTask();
  initIdleStats();
  initDiagnostics();

}

void loop( void ) {
  dnsServer.processNextRequest();
  vTaskDelay( 100 );
  if( digitalRead( sectionRateConfig.gpioManualAutoSelection ) == AOGsectionControl ){
    Serial.println("\nAuto/Manual switch changed, restarting...\n");
    ESP.restart();
  }
}
