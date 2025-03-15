
#include <stdio.h>

#include <ESPUI.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

uint16_t labelLoad;
uint16_t labelRCAppUdpData;
uint16_t labelSectionStatus;
uint16_t labelRateValve;
uint16_t labelRateMeter;
uint16_t labelRateSettings;
uint16_t labelPIDSettings;
uint16_t labelBuildDate;
uint16_t buttonReset;
char downloadFilename[50];

void setResetButtonToRed() {
  ESPUI.getControl( buttonReset )->color = ControlColor::Alizarin;
  ESPUI.updateControl( buttonReset );
}

void initESPUI ( void ) {

  labelLoad = ESPUI.addControl( ControlType::Label, "Load:", "", ControlColor::Turquoise );
  labelRCAppUdpData = ESPUI.addControl( ControlType::Label, "RC App data:", "N/A", ControlColor::Carrot );

  buttonReset = ESPUI.addControl( ControlType::Button, "Store the Settings", "Apply", ControlColor::Carrot, Control::noParent,
  []( Control * control, int id ) {
    if( id == B_UP ) {
      saveConfig();
    }
  } );

  buttonReset = ESPUI.addControl( ControlType::Button, "If this turns red, you have to", "Apply & Reboot", ControlColor::Carrot, Control::noParent,
  []( Control * control, int id ) {
    if( id == B_UP ) {
      saveConfig();
      SPIFFS.end();
      ESP.restart();
    }
  } );

  uint16_t tabConfigurations;

  // Diagnostics Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Diagnostics", "Diagnostics" );

    labelSectionStatus = ESPUI.addControl( ControlType::Label, "Section data:", "N/A", ControlColor::Turquoise, tab );
    labelRateValve = ESPUI.addControl( ControlType::Label, "Rate motor/valve data:", "N/A", ControlColor::Turquoise, tab );
    labelRateMeter = ESPUI.addControl( ControlType::Label, "Rate meter data:", "N/A", ControlColor::Turquoise, tab );
    labelRateSettings = ESPUI.addControl( ControlType::Label, "Rate settings:", "N/A", ControlColor::Turquoise, tab );
    labelPIDSettings = ESPUI.addControl( ControlType::Label, "PID settings:", "N/A", ControlColor::Turquoise, tab );
    String buildDate = String(__DATE__);
    buildDate += String(" ");
    buildDate += String(__TIME__);
    labelBuildDate = ESPUI.addControl( ControlType::Label, "Build date:", buildDate, ControlColor::Turquoise, tab );
  }

  // Network Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Network", "Network" );

    ESPUI.addControl( ControlType::Text, "SSID*", String( sectionRateConfig.ssid ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( sectionRateConfig.ssid, sizeof( sectionRateConfig.ssid ) );
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Text, "Password*", String( sectionRateConfig.password ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( sectionRateConfig.password, sizeof( sectionRateConfig.password ) );
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Text, "Hostname*", String( sectionRateConfig.hostname ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( sectionRateConfig.hostname, sizeof( sectionRateConfig.hostname ) );
      setResetButtonToRed();
    } );

    ESPUI.addControl( ControlType::Switcher, "OTA Enabled*", sectionRateConfig.enableOTA ? "1" : "0", ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      sectionRateConfig.enableOTA = control->value.toInt() == 1;
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Number, "RC Port to send from*", String( sectionRateConfig.rcPortSendFrom ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      sectionRateConfig.rcPortSendFrom = control->value.toInt();
      setResetButtonToRed();
    } );

    ESPUI.addControl( ControlType::Number, "RC Port to send to*", String( sectionRateConfig.rcPortSendTo ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      sectionRateConfig.rcPortSendTo = control->value.toInt();
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Number, "RC Port to listen to*", String( sectionRateConfig.rcPortListenTo ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      sectionRateConfig.rcPortListenTo = control->value.toInt();
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Number, "AOG Port to listen to*", String( sectionRateConfig.aogPortListenTo ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      sectionRateConfig.aogPortListenTo = control->value.toInt();
      setResetButtonToRed();
    } );
  }

  // Settings Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Settings", "Settings" );
    {
      ESPUI.addControl( ControlType::Switcher, "Invert rate motor", sectionRateConfig.invertRateMotor ? "1" : "0", ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        sectionRateConfig.invertRateMotor = control->value.toInt() == 1;
      } );
    }
    {
      ESPUI.addControl( ControlType::Switcher, "Rate control always manual", sectionRateConfig.rateControlAlwaysManual ? "1" : "0", ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        sectionRateConfig.rateControlAlwaysManual = control->value.toInt() == 1;
      } );
    }
  }

  char autosteerDownloadHTML [100];
  sprintf( downloadFilename, "/%s section rate.json", sectionRateConfig.hostname );
  sprintf( autosteerDownloadHTML, "<a href='%s'>Configuration</a>", downloadFilename );

  // Default Configurations Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Configurations", "Configurations" );
  
    ESPUI.addControl( ControlType::Label, "OTA Update:", "<a href='/update'>Update</a>", ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Download the config:", autosteerDownloadHTML, ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Upload the config:", "<form method='POST' action='/upload-config' enctype='multipart/form-data'><input name='f' type='file'><input type='submit'>ESP32 will restart after submitting</form>", ControlColor::Carrot, tab );
    
    tabConfigurations = tab;

  }
  
  static String title;

  title = "Section / Rate :: ";

  title += sectionRateConfig.hostname;
  ESPUI.begin( title.c_str() );

  ESPUI.server->on( downloadFilename, HTTP_GET, []( AsyncWebServerRequest * request ) {
    
    Serial.print( "Preparing " );
    Serial.print( downloadFilename );
    Serial.println( " for download" );

    char ibuffer[64];
    File f1 = SPIFFS.open( "/sectionRate.json", "r" );    //open source file to read
    if ( !f1 ){
      Serial.println( "/sectionRate.json not available for copying" );
      return;
    }

    File f2 = SPIFFS.open( downloadFilename, "w" );    //open destination file to write
    if ( !f2 ){
      Serial.print( downloadFilename );
      Serial.println( " could not be created" );
      return;
    }
    
    uint8_t blocks = 0;
    Serial.print( "Copied " );
    while ( f1.available() > 0 ){
      byte i = f1.readBytes( ibuffer, 64 ); // i = number of bytes placed in buffer from file f1
      f2.write(( uint8_t* )ibuffer, i );    // write i bytes from buffer to file f2
      blocks += 1;
      Serial.print( blocks );
      Serial.print( " " );
    }
    Serial.println( "blocks" );
    
    f2.close();
    f1.close();
    Serial.println( "File creation successful, downloading..." );
    delay( 5 );
    request->send( SPIFFS, downloadFilename, "application/json", true );
    delay( 5 );
    SPIFFS.remove( downloadFilename );
  } );
  
  // upload a file to /upload-config
  ESPUI.server->on( "/upload-config", HTTP_POST, []( AsyncWebServerRequest * request ) {
    request->send( 200 );
  }, [tabConfigurations]( AsyncWebServerRequest * request, String filename, size_t index, uint8_t* data, size_t len, bool final ) {
    if( !index ) {
      request->_tempFile = SPIFFS.open( "/sectionRate.json", "w" );
    }

    if( request->_tempFile ) {
      if( len ) {
        request->_tempFile.write( data, len );
      }

      if( final ) {
        request->_tempFile.close();
        delay(10);
        ESP.restart();
      }
    }
  } );
}
