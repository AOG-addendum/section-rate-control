

#include "main.hpp"
#include "esp_wifi.h"
#include <WiFi.h>

IPAddress softApIP( 192, 168, 1, 1 );
String apName;
bool WiFiWasConnected = false;

void WiFiStationGotIP( WiFiEvent_t event, WiFiEventInfo_t info ){
  IPAddress myIP = WiFi.localIP();
  if( myIP[3] != 71 ){
      myIP[3] = 71;
      IPAddress gwIP = WiFi.gatewayIP();
      if ( !WiFi.config( myIP, gwIP, IPAddress( 255, 255, 255, 0 ), gwIP )) {
        Serial.println("STA Failed to configure");
      }
      ipDestination = myIP;
      ipDestination[3] = 255;
      Serial.println("Switching off AP, station only");
      WiFi.softAPdisconnect ( true );
      WiFi.mode( WIFI_MODE_STA );
  }
  delay( 10 );
  digitalWrite( sectionRateConfig.gpioWifiLed, HIGH );
  WiFiWasConnected = true;
}

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info ){
  digitalWrite( sectionRateConfig.gpioWifiLed, !sectionRateConfig.WifiLedOnLevel );
  if( WiFiWasConnected == true ){
    WiFi.disconnect( true );
    if( !WiFi.config( INADDR_NONE, INADDR_NONE, INADDR_NONE ) ) {
      Serial.println("STA Failed to unset configuration");
    }
    delay( 25 );
    WiFi.begin( sectionRateConfig.ssid, sectionRateConfig.password );
    Serial.println("reconnecting");
  } else {
    WiFi.reconnect();
  }
}

void WiFiStationConnected( WiFiEvent_t event, WiFiEventInfo_t info ){
  WiFi.config( INADDR_NONE, INADDR_NONE, INADDR_NONE );
  WiFi.setHostname( sectionRateConfig.hostname );
  delay( 10 );
  digitalWrite( sectionRateConfig.gpioWifiLed, HIGH );
}

void WiFiAPStaConnected( WiFiEvent_t event, WiFiEventInfo_t info ){
  Serial.println( "Switching off station mode, AP only" );
  WiFi.disconnect( true );
  delay( 100 );
  WiFi.mode( WIFI_MODE_AP );
}

void initWiFi( void ){
  delay( 50 );
  WiFi.config( INADDR_NONE, INADDR_NONE, INADDR_NONE );
  delay( 50 );
  WiFi.onEvent( WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED );
  WiFi.onEvent( WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED );
  WiFi.onEvent( WiFiStationGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP );
  WiFi.onEvent( WiFiAPStaConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED );
  // try to connect to existing network
  WiFi.begin( sectionRateConfig.ssid, sectionRateConfig.password );
  WiFi.setAutoReconnect ( false );
  Serial.print( "\n\nTry to connect to existing network \"" );
  Serial.print( sectionRateConfig.ssid );
  Serial.print( "\" with password \"" );
  Serial.print( sectionRateConfig.password );
  Serial.println( "\"" );

  uint8_t timeout = 5;
  // Wait for connection, 2.5s timeout
  do {
  delay( 500 );
  Serial.print( "." );
  digitalWrite( sectionRateConfig.gpioWifiLed, ! digitalRead( sectionRateConfig.gpioWifiLed ));
  timeout--;
  } while( timeout && WiFi.status() != WL_CONNECTED );
  // not connected -> create hotspot
  if( WiFi.status() != WL_CONNECTED ) {
      WiFi.disconnect( true );

      digitalWrite( sectionRateConfig.gpioWifiLed, LOW );

      apName = String( "Section - Rate control " );
      apName += WiFi.macAddress();
      apName.replace( ":", "" );

      Serial.print( "\n\nCreating hotspot \"" );
      Serial.print( apName.c_str() );
      Serial.println( "\"" );
      WiFi.mode( WIFI_MODE_APSTA );
      delay( 25 );
      WiFi.softAP( apName.c_str() );
      WiFi.begin( sectionRateConfig.ssid, sectionRateConfig.password );
      while ( !SYSTEM_EVENT_AP_START ){ // wait until AP has started
          delay( 100);
          Serial.print(".");
      }
      WiFi.softAPConfig( softApIP, softApIP, IPAddress( 255, 255, 255, 0 ) );
      delay( 25 );
    }
}
