
#include <stdio.h>

#include <ESPUI.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

uint16_t labelLoad;
uint16_t labelRCAppUdpData;
uint16_t labelSectionStatus;
uint16_t labelRateMotor;
uint16_t buttonReset;

void setResetButtonToRed() {
  ESPUI.getControl( buttonReset )->color = ControlColor::Alizarin;
  ESPUI.updateControlAsync( buttonReset );
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
    labelRateMotor = ESPUI.addControl( ControlType::Label, "Rate motor data:", "N/A", ControlColor::Turquoise, tab );
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

  // Default Configurations Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Configurations", "Configurations" );
    ESPUI.addControl( ControlType::Label, "Attention:", "These Buttons here reset the whole config. This affects the WIFI too, if not configured otherwise below. You have to press \"Apply & Reboot\" above to actualy store them.", ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "OTA Update:", "<a href='/update'>Update</a>", ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Download the config:", "<a href='config.json'>Configuration</a>", ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Upload the config:", "<form method='POST' action='/upload-config' enctype='multipart/form-data'><input name='f' type='file'><input type='submit'></form>", ControlColor::Carrot, tab );

    // onchange='this.form.submit()'
    {
      ESPUI.addControl( ControlType::Switcher, "Retain WIFI settings", sectionRateConfig.retainWifiSettings ? "1" : "0", ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        sectionRateConfig.retainWifiSettings = control->value.toInt() == 1;
      } );
    }
    {
      ESPUI.addControl( ControlType::Button, "Set Settings To Default*", "Defaults", ControlColor::Wetasphalt, tab,
      []( Control * control, int id ) {
        char ssid[24], password[24], hostname[24];

        if( sectionRateConfig.retainWifiSettings ) {
          memcpy( ssid, sectionRateConfig.ssid, sizeof( ssid ) );
          memcpy( password, sectionRateConfig.password, sizeof( password ) );
          memcpy( hostname, sectionRateConfig.hostname, sizeof( hostname ) );
        }

        sectionRateConfig = sectionRateConfigDefaults;

        if( sectionRateConfig.retainWifiSettings ) {
          memcpy( sectionRateConfig.ssid, ssid, sizeof( ssid ) );
          memcpy( sectionRateConfig.password, password, sizeof( password ) );
          memcpy( sectionRateConfig.hostname, hostname, sizeof( hostname ) );
        }

        setResetButtonToRed();
      } );
    }

    tabConfigurations = tab;

  }
  
  static String title;

  title = "Section / Rate :: ";

  title += sectionRateConfig.hostname;
  ESPUI.begin( title.c_str() );

  ESPUI.server->on( "/config.json", HTTP_GET, []( AsyncWebServerRequest * request ) {
    request->send( SPIFFS, "/config.json", "application/json", true );
  } );
  
  // upload a file to /upload-config
  ESPUI.server->on( "/upload-config", HTTP_POST, []( AsyncWebServerRequest * request ) {
    request->send( 200 );
  }, [tabConfigurations]( AsyncWebServerRequest * request, String filename, size_t index, uint8_t* data, size_t len, bool final ) {
    if( !index ) {
      request->_tempFile = SPIFFS.open( "/config.json", "w" );
    }

    if( request->_tempFile ) {
      if( len ) {
        request->_tempFile.write( data, len );
      }

      if( final ) {
        request->_tempFile.close();
        setResetButtonToRed();
        String str( "/#tab" );
        str += tabConfigurations;
        request->redirect( str );
      }
    }
  } );
}
