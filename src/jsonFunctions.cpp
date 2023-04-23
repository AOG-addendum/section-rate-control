

#include <memory>

#include <FS.h>
#include <SPIFFS.h>


#include "main.hpp"
#include "jsonFunctions.hpp"

void loadSavedConfig() {
  {
    auto j = loadJsonFromFile( "/config.json" );
    parseJsonToSectionRateConfig( j, sectionRateConfig );
  }
}

void saveConfig() {
  {
    const auto j = parseSectionRateConfigToJson( sectionRateConfig );
    saveJsonToFile( j, "/config.json" );
  }
}

void loadPIDConfig() {
  {
    auto j = loadJsonFromFile( "/pidConfig.json" );
    parseJsonToPIDConfig( j, pidConfig );
  }
}

void savePIDConfig() {
  {
    const auto j = parsePIDConfigToJson( pidConfig );
    saveJsonToFile( j, "/pidConfig.json" );
  }
}

json loadJsonFromFile( const char* fileName ) {
  json j;

  if( SPIFFS.exists( fileName ) ) {
    File file = SPIFFS.open( fileName, "r" );

    if( file ) {
      std::vector<uint8_t> data;
      data.resize( file.size() );

      file.read( data.data(), file.size() );

      try {
        j = json::parse( data/*, nullptr, false*/ );
      } catch( json::exception& e ) {
        // output exception information
        Serial.print( "message: " );
        Serial.println( e.what() );
        Serial.print( "exception id: " );
        Serial.println( e.id );
      }
    } else {
      Serial.print( "Could not open file for reading: " );
      Serial.println( fileName );
      Serial.flush();
    }

    file.close();
  }

  return j;
}

void saveJsonToFile( const json& json, const char* fileName ) {
  // pretty print with 2 spaces indentation
  auto data = json.dump( 2 );

  File file = SPIFFS.open( fileName, "w" );

  if( file && !file.isDirectory() ) {
    file.write( ( uint8_t* )data.c_str(), data.size() );
  } else {
    Serial.print( "Could not open file for writing: " );
    Serial.println( fileName );
    Serial.flush();
  }

  file.close();
}

json parseSectionRateConfigToJson( const SectionRateConfig& config ) {
  json j;

  j["wifi"]["ssid"] = config.ssid;
  j["wifi"]["password"] = config.password;
  j["wifi"]["hostname"] = config.hostname;
  j["wifi"]["retainSettings"] = config.retainWifiSettings;

  j["connection"]["baudrate"] = config.baudrate;
  j["connection"]["enableOTA"] = config.enableOTA;

  j["connection"]["rc"]["sendFrom"] = config.rcPortSendFrom;
  j["connection"]["rc"]["listenTo"] = config.rcPortListenTo;
  j["connection"]["rc"]["sendTo"] = config.rcPortSendTo;
  j["connection"]["aog"]["listenTo"] = config.aogPortListenTo;

  j["settings"]["invertRateMotor"] = config.invertRateMotor;
  return j;
}

void parseJsonToSectionRateConfig( json& j, SectionRateConfig& config ) {
  if( j.is_object() ) {
    try {
      {
        std::string str = j.value( "/wifi/ssid"_json_pointer, sectionRateConfigDefaults.ssid );
        memset( config.ssid, 0, sizeof( config.ssid ) );
        memcpy( config.ssid, str.c_str(), str.size() );
      }
      {
        std::string str = j.value( "/wifi/password"_json_pointer, sectionRateConfigDefaults.password );
        memset( config.password, 0, sizeof( config.password ) );
        memcpy( config.password, str.c_str(), str.size() );
      }
      {
        std::string str = j.value( "/wifi/hostname"_json_pointer, sectionRateConfigDefaults.hostname );
        memset( config.hostname, 0, sizeof( config.hostname ) );
        memcpy( config.hostname, str.c_str(), str.size() );
      }
      config.retainWifiSettings = j.value( "/wifi/retainSettings"_json_pointer, sectionRateConfigDefaults.retainWifiSettings );

      config.baudrate = j.value( "/connection/baudrate"_json_pointer, sectionRateConfigDefaults.baudrate );
      config.enableOTA = j.value( "/connection/enableOTA"_json_pointer, sectionRateConfigDefaults.enableOTA );

      config.rcPortSendFrom = j.value( "/connection/rc/sendFrom"_json_pointer, sectionRateConfigDefaults.rcPortSendFrom );
      config.rcPortListenTo = j.value( "/connection/rc/listenTo"_json_pointer, sectionRateConfigDefaults.rcPortListenTo );
      config.rcPortSendTo = j.value( "/connection/rc/sendTo"_json_pointer, sectionRateConfigDefaults.rcPortSendTo );
      config.aogPortListenTo = j.value( "/connection/aog/listenTo"_json_pointer, sectionRateConfigDefaults.aogPortListenTo );

      config.invertRateMotor = j.value( "/settings/invertRateMotor"_json_pointer, sectionRateConfigDefaults.invertRateMotor );
    } catch( json::exception& e ) {
      // output exception information
      Serial.print( "message: " );
      Serial.println( e.what() );
      Serial.print( "exception id: " );
      Serial.println( e.id );
      Serial.flush();
    }
  }
}

json parsePIDConfigToJson( const PIDConfig& config ) {
  json j;

  j["PID"]["KP"] = config.KP;
  j["PID"]["KI"] = config.KI;

  j["limit"]["MinPWM"] = config.MinPWM;
  j["limit"]["LowMax"] = config.LowMax;
  j["limit"]["HighMax"] = config.HighMax;
  j["limit"]["Deadband"] = config.Deadband;
  j["limit"]["BrakePoint"] = config.BrakePoint;
  j["limit"]["AdjustTime"] = config.AdjustTime;

  return j;
}

void parseJsonToPIDConfig( json& j, PIDConfig& config ) {
  if( j.is_object() ) {
    try {
      config.KP = j.value( "/PID/KP"_json_pointer, pidConfigDefaults.KP );
      config.KI = j.value( "/PID/KI"_json_pointer, pidConfigDefaults.KI );

      config.MinPWM = j.value( "/limit/MinPWM"_json_pointer, pidConfigDefaults.MinPWM );
      config.LowMax = j.value( "/limit/LowMax"_json_pointer, pidConfigDefaults.LowMax );
      config.HighMax = j.value( "/limit/HighMax"_json_pointer, pidConfigDefaults.HighMax );
      config.Deadband = j.value( "/limit/Deadband"_json_pointer, pidConfigDefaults.Deadband );
      config.BrakePoint = j.value( "/limit/BrakePoint"_json_pointer, pidConfigDefaults.BrakePoint );
      config.AdjustTime = j.value( "/limit/AdjustTime"_json_pointer, pidConfigDefaults.AdjustTime );

    } catch( json::exception& e ) {
      // output exception information
      Serial.print( "message: " );
      Serial.println( e.what() );
      Serial.print( "exception id: " );
      Serial.println( e.id );
      Serial.flush();
    }
  }
}