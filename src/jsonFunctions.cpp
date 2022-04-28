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

#include <memory>

#include <FS.h>
#include <SPIFFS.h>

extern "C" {
#include <crypto/base64.h>
}

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

void loadSavedRuntimeData() {
  {
    auto j = loadJsonFromFile( "/runtimeData.json" );
    parseJsonToRuntimeData( j, runtimeData );
  }
}

void saveRuntimeData() {
  {
    const auto j = parseRuntimeDataToJson( runtimeData );
    saveJsonToFile( j, "/runtimeData.json" );
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
  j["wifi"]["apModePin"] = int( config.apModePin );
  j["wifi"]["retainSettings"] = config.retainWifiSettings;

  j["rate"]["gpioFlowSensorEncoder"] = config.gpioFlowSensorEncoder;
  j["rate"]["pulsesPerUnit"] = config.pulsesPerUnit;
  j["rate"]["targetFlowRate"] = config.targetFlowRate;
  j["rate"]["flowRateOffset"] = config.flowRateOffset;
  j["rate"]["tankSize"] = config.tankSize;

  j["sections"]["gpioSection1"] = config.gpioSection1;
  j["sections"]["gpioSection2"] = config.gpioSection2;
  j["sections"]["gpioSection3"] = config.gpioSection3;
  j["sections"]["gpioSection4"] = config.gpioSection4;
  j["sections"]["gpioSection5"] = config.gpioSection5;
  j["sections"]["gpioSection6"] = config.gpioSection6;
  j["sections"]["gpioSection7"] = config.gpioSection7;
  j["sections"]["gpioSection8"] = config.gpioSection8;

  j["sections"]["widthSection1"] = config.widthSection1;
  j["sections"]["widthSection2"] = config.widthSection2;
  j["sections"]["widthSection3"] = config.widthSection3;
  j["sections"]["widthSection4"] = config.widthSection4;
  j["sections"]["widthSection5"] = config.widthSection5;
  j["sections"]["widthSection6"] = config.widthSection6;
  j["sections"]["widthSection7"] = config.widthSection7;
  j["sections"]["widthSection8"] = config.widthSection8;

  j["output"]["outputType"] = int( config.outputType );
  j["output"]["rateControlType"] = int( config.rateControlType );
  j["output"]["pwmFrequency"] = config.pwmFrequency;
  j["output"]["minPWM"] = config.rateControlMinPwm;
  j["output"]["gpioPwm"] = int( config.gpioPwm );
  j["output"]["gpioDir"] = int( config.gpioDir );
  j["output"]["gpioEn"] = int( config.gpioEn );
  j["output"]["invertOutput"] = config.invertOutput;
  j["output"]["outputOnTime"] = config.outputOnTime;

  j["PID"]["P"] = config.rateControlPidKp;
  j["PID"]["I"] = config.rateControlPidKi;
  j["PID"]["IMax"] = config.rateControlPidKiMax;
  j["PID"]["D"] = config.rateControlPidKd;

  j["i2c"]["sda"] = int( config.gpioSDA );
  j["i2c"]["scl"] = int( config.gpioSCL );
  j["i2c"]["speed"] = config.i2cBusSpeed;

  j["autoManual"]["gpioAutoManualControl"] = int( config.gpioAutoManualControl );
  j["autoManual"]["invertAutoManual"] = config.invertAutoManual;

  j["measurement"]["unitType"] = config.unitType;

  j["connection"]["baudrate"] = config.baudrate;
  j["connection"]["enableOTA"] = config.enableOTA;

  j["connection"]["aog"]["sendFrom"] = config.aogPortSendFrom;
  j["connection"]["aog"]["listenTo"] = config.aogPortListenTo;
  j["connection"]["aog"]["sendTo"] = config.aogPortSendTo;

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
      config.apModePin = j.value( "/wifi/apModePin"_json_pointer, sectionRateConfigDefaults.apModePin );
      config.retainWifiSettings = j.value( "/wifi/retainSettings"_json_pointer, sectionRateConfigDefaults.retainWifiSettings );

      config.gpioFlowSensorEncoder = j.value( "/rate/gpioFlowSensorEncoder"_json_pointer, sectionRateConfigDefaults.gpioFlowSensorEncoder );
      config.pulsesPerUnit = j.value( "/rate/pulsesPerUnit"_json_pointer, sectionRateConfigDefaults.pulsesPerUnit );
      config.targetFlowRate = j.value( "/rate/targetFlowRate"_json_pointer, sectionRateConfigDefaults.targetFlowRate );
      config.flowRateOffset = j.value( "/rate/flowRateOffset"_json_pointer, sectionRateConfigDefaults.flowRateOffset );
      config.tankSize = j.value( "/rate/tankSize"_json_pointer, sectionRateConfigDefaults.tankSize );

      config.gpioSection1 = j.value( "/sections/gpioSection1"_json_pointer, sectionRateConfigDefaults.gpioSection1 );
      config.gpioSection2 = j.value( "/sections/gpioSection2"_json_pointer, sectionRateConfigDefaults.gpioSection2 );
      config.gpioSection3 = j.value( "/sections/gpioSection3"_json_pointer, sectionRateConfigDefaults.gpioSection3 );
      config.gpioSection4 = j.value( "/sections/gpioSection4"_json_pointer, sectionRateConfigDefaults.gpioSection4 );
      config.gpioSection5 = j.value( "/sections/gpioSection5"_json_pointer, sectionRateConfigDefaults.gpioSection5 );
      config.gpioSection6 = j.value( "/sections/gpioSection6"_json_pointer, sectionRateConfigDefaults.gpioSection6 );
      config.gpioSection7 = j.value( "/sections/gpioSection7"_json_pointer, sectionRateConfigDefaults.gpioSection7 );
      config.gpioSection8 = j.value( "/sections/gpioSection8"_json_pointer, sectionRateConfigDefaults.gpioSection8 );

      config.widthSection1 = j.value( "/sections/widthSection1"_json_pointer, sectionRateConfigDefaults.widthSection1 );
      config.widthSection2 = j.value( "/sections/widthSection2"_json_pointer, sectionRateConfigDefaults.widthSection2 );
      config.widthSection3 = j.value( "/sections/widthSection3"_json_pointer, sectionRateConfigDefaults.widthSection3 );
      config.widthSection4 = j.value( "/sections/widthSection4"_json_pointer, sectionRateConfigDefaults.widthSection4 );
      config.widthSection5 = j.value( "/sections/widthSection5"_json_pointer, sectionRateConfigDefaults.widthSection5 );
      config.widthSection6 = j.value( "/sections/widthSection6"_json_pointer, sectionRateConfigDefaults.widthSection6 );
      config.widthSection7 = j.value( "/sections/widthSection7"_json_pointer, sectionRateConfigDefaults.widthSection7 );
      config.widthSection8 = j.value( "/sections/widthSection8"_json_pointer, sectionRateConfigDefaults.widthSection8 );

      config.outputType = j.value( "/output/outputType"_json_pointer, sectionRateConfigDefaults.outputType );
      config.rateControlType = j.value( "/output/rateControlType"_json_pointer, sectionRateConfigDefaults.rateControlType );
      config.pwmFrequency = j.value( "/output/pwmFrequency"_json_pointer, sectionRateConfigDefaults.pwmFrequency );
      config.rateControlMinPwm = j.value( "/output/minPWM"_json_pointer, sectionRateConfigDefaults.rateControlMinPwm );
      config.gpioPwm = j.value( "/output/gpioPwm"_json_pointer, sectionRateConfigDefaults.gpioPwm );
      config.gpioDir = j.value( "/output/gpioDir"_json_pointer, sectionRateConfigDefaults.gpioDir );
      config.gpioEn = j.value( "/output/gpioEn"_json_pointer, sectionRateConfigDefaults.gpioEn );
      config.invertOutput = j.value( "/output/invertOutput"_json_pointer, sectionRateConfigDefaults.invertOutput );
      config.outputOnTime = j.value( "/output/outputOnTime"_json_pointer, sectionRateConfigDefaults.outputOnTime );

      config.rateControlPidKp = j.value( "/PID/P"_json_pointer, sectionRateConfigDefaults.rateControlPidKp );
      config.rateControlPidKi = j.value( "/PID/I"_json_pointer, sectionRateConfigDefaults.rateControlPidKi );
      config.rateControlPidKiMax = j.value( "/PID/IMax"_json_pointer, sectionRateConfigDefaults.rateControlPidKiMax );
      config.rateControlPidKd = j.value( "/PID/D"_json_pointer, sectionRateConfigDefaults.rateControlPidKd );

      config.gpioSDA = j.value( "/i2c/sda"_json_pointer, sectionRateConfigDefaults.gpioSDA );
      config.gpioSCL = j.value( "/i2c/scl"_json_pointer, sectionRateConfigDefaults.gpioSCL );
      config.i2cBusSpeed = j.value( "/i2c/speed"_json_pointer, sectionRateConfigDefaults.i2cBusSpeed );

      config.gpioAutoManualControl = j.value( "/autoManual/gpioAutoManualControl"_json_pointer, sectionRateConfigDefaults.gpioAutoManualControl );
      config.invertAutoManual = j.value( "/autoManual/invertAutoManual"_json_pointer, sectionRateConfigDefaults.invertAutoManual );

      config.unitType = j.value( "/measurement/unitType"_json_pointer, sectionRateConfigDefaults.unitType );

      config.baudrate = j.value( "/connection/baudrate"_json_pointer, sectionRateConfigDefaults.baudrate );
      config.enableOTA = j.value( "/connection/enableOTA"_json_pointer, sectionRateConfigDefaults.enableOTA );

      config.aogPortSendFrom = j.value( "/connection/aog/sendFrom"_json_pointer, sectionRateConfigDefaults.aogPortSendFrom );
      config.aogPortListenTo = j.value( "/connection/aog/listenTo"_json_pointer, sectionRateConfigDefaults.aogPortListenTo );
      config.aogPortSendTo = j.value( "/connection/aog/sendTo"_json_pointer, sectionRateConfigDefaults.aogPortSendTo );

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

json parseRuntimeDataToJson( const RuntimeData& runtimeData ) {
  json j;

  j["totalFluidUnitsApplied"] = runtimeData.totalFluidUnitsApplied;
  j["totalLandUnitsApplied"] = runtimeData.totalLandUnitsApplied;
  j["tankFluidRemaining"] = runtimeData.tankFluidRemaining;
  j["areaCoverage"] = runtimeData.areaCoverage;
  j["fluidAmount"] = runtimeData.fluidAmount;
  j["currentDisplayIndex"] = runtimeData.currentDisplayIndex;

  return j;
}

void parseJsonToRuntimeData( json& j, RuntimeData& runtimeData ) {
  if( j.is_object() ) {
    try {

      runtimeData.totalFluidUnitsApplied = j.value( "/totalFluidUnitsApplied"_json_pointer, runtimeData.totalFluidUnitsApplied );
      runtimeData.totalLandUnitsApplied = j.value( "/totalLandUnitsApplied"_json_pointer, runtimeData.totalLandUnitsApplied );
      runtimeData.tankFluidRemaining = j.value( "/tankFluidRemaining"_json_pointer, runtimeData.tankFluidRemaining );
      runtimeData.areaCoverage = j.value( "/areaCoverage"_json_pointer, runtimeData.areaCoverage );
      runtimeData.fluidAmount = j.value( "/fluidAmount"_json_pointer, runtimeData.fluidAmount );
      runtimeData.currentDisplayIndex = j.value( "/currentDisplayIndex"_json_pointer, runtimeData.currentDisplayIndex );

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

void sendBase64DataTransmission( uint16_t channelId, const char* data, size_t len ) {
  json j;
  j["channelId"] = channelId;

  size_t outputLength;
  char* encoded = ( char* )base64_encode( ( const unsigned char* )data, len, &outputLength );

  if( encoded ) {
    j["data"] = std::string( encoded, outputLength );
    free( encoded );

    std::vector<std::uint8_t> cbor = json::to_cbor( j );
    udpSendFrom.broadcastTo( cbor.data(), cbor.size(), initialisation.portSendTo );
  }
}

void sendStateTransmission( uint16_t channelId, bool state ) {
  json j;
  j["channelId"] = channelId;
  j["state"] = state;

  std::vector<std::uint8_t> cbor = json::to_cbor( j );
  udpSendFrom.broadcastTo( cbor.data(), cbor.size(), initialisation.portSendTo );
}

void sendNumberTransmission( uint16_t channelId, double number ) {
  json j;
  j["channelId"] = channelId;
  j["number"] = number;

  std::vector<std::uint8_t> cbor = json::to_cbor( j );
  udpSendFrom.broadcastTo( cbor.data(), cbor.size(), initialisation.portSendTo );
}
