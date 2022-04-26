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

#include <AutoPID.h>

#include "main.hpp"
#include "jsonFunctions.hpp"
#include "display.hpp"

#include <string>       // std::string
#include <sstream>      // std::stringstream


SectionSetpoints sectionSetpoints;
TravelData travelData;
RuntimeData runtimeData;

AsyncUDP udpSendFrom;
AsyncUDP udpLocalPort;
AsyncUDP udpRemotePort;

double pidOutput = 0;
double actualFlowRate = 0.00;
double pidOutputTmp;
uint16_t outputOnTime;
AutoPID pid(
        &( actualFlowRate ),
        &( sectionRateConfig.targetFlowRate ),
        &( pidOutput ),
        -255, 255,
        sectionRateConfig.rateControlPidKp, sectionRateConfig.rateControlPidKi, sectionRateConfig.rateControlPidKd );

JsonQueueSelector jsonQueueSelector;

constexpr time_t Timeout = 1000;
time_t timeoutPoint = 0;
volatile uint32_t flowSensorPulseCount = 0; // 32 bits for fast operation

const uint8_t coverageElements = 30; // beware of changes in array size, as this will affect the 30 seconds calculation window
uint8_t coverageIndex = 0;
uint32_t coverageArray[coverageElements][2] = {0, 0};
uint64_t coverageArrayTotalArea = 0;

uint64_t totalEncoderCount = 0;
uint64_t oldestEncoderCount = 0;

void rateDisplayWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for( ;; ) {

    totalEncoderCount += flowSensorPulseCount;  // process flow meter even if we're not moving
    uint32_t amount = flowSensorPulseCount / ( double ) sectionRateConfig.pulsesPerUnit;
    flowSensorPulseCount = 0;
    runtimeData.tankFluidRemaining -= amount;
    runtimeData.totalFluidUnitsApplied += amount;

    if( travelData.clearData == false &&     // do not process stale data
        sectionSetpoints.enabled == true ){
      travelData.clearData = true; // tell the code receiving UDP info from AOG to clear data
      coverageArrayTotalArea -= coverageArray[coverageIndex][0];
      runtimeData.areaCoverage = ( travelData.squareUnitsCovered / ( float )travelData.counts );
      runtimeData.totalLandUnitsApplied += runtimeData.areaCoverage;
      coverageArray[coverageIndex][0] = runtimeData.areaCoverage;
      coverageArray[coverageIndex][1] = totalEncoderCount;
      coverageArrayTotalArea += coverageArray[coverageIndex][0];
      coverageIndex += 1;
      if( coverageIndex >= coverageElements ){
        coverageIndex = 0;
      }
      oldestEncoderCount = coverageArray[coverageIndex][1]; // fluid amount 30 seconds ago
      runtimeData.fluidAmount = ( totalEncoderCount - oldestEncoderCount ) / ( double ) sectionRateConfig.pulsesPerUnit;
      if( runtimeData.fluidAmount == 0 || coverageArrayTotalArea == 0 ){
        actualFlowRate = 0;
      } else {
        if( ( SectionRateConfig::UnitType )sectionRateConfig.unitType == SectionRateConfig::UnitType::Imperial ) {
          actualFlowRate = runtimeData.fluidAmount / (( coverageArrayTotalArea / 43560.0 ) * 0.001 ); // 43560 square feet per acre
        } else {
          actualFlowRate = runtimeData.fluidAmount / (( coverageArrayTotalArea / 10000.0 ) * 0.001 ); // 10000 square meters per hectare
        }
      }
      actualFlowRate *= sectionRateConfig.flowRateOffset;
    }

    DISPLAY_VARIABLES[0] = actualFlowRate;
    DISPLAY_VARIABLES[1] = runtimeData.totalFluidUnitsApplied;
    DISPLAY_VARIABLES[2] = travelData.speed * 0.001;
    DISPLAY_VARIABLES[3] = ( runtimeData.totalLandUnitsApplied / 43560 ) * 0.001;
    DISPLAY_VARIABLES[4] = runtimeData.tankFluidRemaining;
    updateDisplayVariables();
    saveRuntimeData();

    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void rateControlWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // initialize array for quicker accuracy at startup
  // runtimeData.areaCoverage has a totalizing filter method, all values are similar at constant speed
  // runtimeData.fluidAmount has a 30 second differential basis, the value in the next row/index should be GPM / 2 greater
  totalEncoderCount = runtimeData.fluidAmount * sectionRateConfig.pulsesPerUnit;
  for( int i = 0; i < coverageElements; i++ ){
    coverageArrayTotalArea += runtimeData.areaCoverage;
    coverageArray[i][0] = runtimeData.areaCoverage;
    coverageArray[i][1] = ( totalEncoderCount / coverageElements ) * i;

  }

  pid.setTimeStep( xFrequency );

  for( ;; ) {

    // check for timeout, data from AgOpenGPS, speed greater than zero, and > one section enabled
    if( sectionSetpoints.lastPacketReceived < timeoutPoint ||
        sectionSetpoints.enabled == false ) {
      ledcWrite( 0, 0 );
      ledcWrite( 1, 0 );
      pidOutputTmp = 0.00;
    } else {

      pid.setGains( sectionRateConfig.rateControlPidKp, sectionRateConfig.rateControlPidKi, sectionRateConfig.rateControlPidKd );

      if( pid.getIntegral() > 0 && pid.getIntegral() > sectionRateConfig.rateControlPidKiMax ){
        pid.setIntegral(sectionRateConfig.rateControlPidKiMax);
      }
      if( -pid.getIntegral() > 0 && -pid.getIntegral() > sectionRateConfig.rateControlPidKiMax ){
        pid.setIntegral(-sectionRateConfig.rateControlPidKiMax);
      }

      // here comes the magic: executing the PID loop
      // the values are given by pointers, so the AutoPID gets them automatically
      pid.run();

      if( pidOutput ) {

        pidOutputTmp = sectionRateConfig.invertOutput ? pidOutput : -pidOutput;

        if( pidOutputTmp < 0 && pidOutputTmp > -sectionRateConfig.rateControlPidMinPwm ) {
          pidOutputTmp = -sectionRateConfig.rateControlPidMinPwm;
        }

        if( pidOutputTmp > 0 && pidOutputTmp < sectionRateConfig.rateControlPidMinPwm ) {
          pidOutputTmp = sectionRateConfig.rateControlPidMinPwm;
        }

        switch( initialisation.outputType ) {
          case SectionRateConfig::OutputType::TB67H410:
          case SectionRateConfig::OutputType::HBridge: {
            if( pidOutputTmp >= 0 ) {
              ledcWrite( 0, pidOutputTmp );
              ledcWrite( 1, 0 );
              outputOnTime = pidOutputTmp * sectionRateConfig.outputOnTime;
              vTaskDelay( outputOnTime / portTICK_PERIOD_MS );
              ledcWrite( 0, 0 );
            }

            if( pidOutputTmp < 0 ) {
              ledcWrite( 0, 0 );
              ledcWrite( 1, -pidOutputTmp );
              outputOnTime = -pidOutputTmp * sectionRateConfig.outputOnTime;
              vTaskDelay( outputOnTime / portTICK_PERIOD_MS );
              ledcWrite( 1, 0 );
            }

          }
          break;

          case SectionRateConfig::OutputType::IBT2: {
            if( pidOutputTmp >= 0 ) {
              ledcWrite( 1, 255 );
            } else {
              ledcWrite( 0, 255 );
              pidOutputTmp = -pidOutputTmp;
            }
            uint16_t outputOnTime = pidOutputTmp * sectionRateConfig.outputOnTime;
            vTaskDelay( outputOnTime / portTICK_PERIOD_MS );
            ledcWrite( 0, 0 );
            ledcWrite( 1, 0 );

          }
          break;

          default:
            break;
        }
      } else {
        ledcWrite( 0, 0 );
        ledcWrite( 1, 0 );
      }
    }

    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void rateESPUIWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for( ;; ) {
    timeoutPoint = millis() - Timeout;

    switch( sectionRateConfig.rateControlType ) {
      case SectionRateConfig::RateControlType::Valve: {
        Control* labelStatusRateControlTypeHandle = ESPUI.getControl( labelStatusRateControlType );
        String str;
        str.reserve( 30 );
        str = "Valve (Diverting or Blocking), Output: ";
        str += ( double )pidOutputTmp;
        str += "\nOn Time (milliseconds): ";
        str += ( uint16_t )outputOnTime;
        labelStatusRateControlTypeHandle->value = str;
        labelStatusRateControlTypeHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelStatusRateControlTypeHandle );
      }
      break;

      case SectionRateConfig::RateControlType::Motor: {
        Control* labelStatusRateControlTypeHandle = ESPUI.getControl( labelStatusRateControlType );
        String str;
        str.reserve( 30 );
        str = "DC Pump Motor, Output: ";
        str += ( double )pidOutputTmp;
        str += "\nOn Time (milliseconds): ";
        str += ( uint16_t )outputOnTime;
        labelStatusRateControlTypeHandle->value = str;
        labelStatusRateControlTypeHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelStatusRateControlTypeHandle );
      }
      break;

      default:
        break;
    }

    {
      switch( sectionRateConfig.outputType ) {
        case SectionRateConfig::OutputType::TB67H410: {
          Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );
          String str;
          str.reserve( 30 );
          str = "TB67H410, Target: ";
          str += ( float )sectionRateConfig.targetFlowRate;
          str += ", Actual: ";
          str += ( float )actualFlowRate;
          str += ", timeout: ";
          str += ( bool )( sectionSetpoints.lastPacketReceived < timeoutPoint ) ? "Yes" : "No" ;
          labelStatusOutputTypeHandle->value = str;
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
        }
        break;

        case SectionRateConfig::OutputType::HBridge: {
          Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );
          String str;
          str.reserve( 30 );
          str = "HBridge, Target: ";
          str += ( float )sectionRateConfig.targetFlowRate;
          str += ", Actual: ";
          str += ( float )actualFlowRate;
          str += ", timeout: ";
          str += ( bool )( sectionSetpoints.lastPacketReceived < timeoutPoint ) ? "Yes" : "No" ;
          labelStatusOutputTypeHandle->value = str;
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
        }
        break;

        case SectionRateConfig::OutputType::IBT2: {
          Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );
          String str;
          str.reserve( 30 );
          str = "IBT2, Target: ";
          str += ( float )sectionRateConfig.targetFlowRate;
          str += ", Actual: ";
          str += ( float )actualFlowRate;
          str += ", timeout: ";
          str += ( bool )( sectionSetpoints.lastPacketReceived < timeoutPoint ) ? "Yes" : "No" ;
          labelStatusOutputTypeHandle->value = str;
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
        }
        break;

        default:
          break;

        }
        if( sectionRateConfig.gpioFlowSensorEncoder != SectionRateConfig::Gpio::None ){
          Control* labelStatusMeterHandle = ESPUI.getControl( labelStatusMeter );
          String str;
          str.reserve( 30 );
          str = "Encoder Pulses: ";
          str += ( uint32_t )totalEncoderCount;
          if( ( SectionRateConfig::UnitType )sectionRateConfig.unitType == SectionRateConfig::UnitType::Imperial ) {
            str += ", Gallons: ";
          } else {
            str += ", Liters: ";
          }
          str += ( double )( ( double ) totalEncoderCount / ( double ) sectionRateConfig.pulsesPerUnit );
          labelStatusMeterHandle->value = str;
          labelStatusMeterHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusMeterHandle );
        }
      }

    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void IRAM_ATTR flowSensorISR() {
    // interrupt service routine for the flow sensor
    flowSensorPulseCount += 1;
}

void initRateControl() {

  {
    if( sectionRateConfig.gpioPwm != SectionRateConfig::Gpio::None ) {
      pinMode( ( uint8_t )sectionRateConfig.gpioPwm, OUTPUT );
          ledcSetup( 0, ( uint8_t )sectionRateConfig.pwmFrequency, 8 );
          ledcAttachPin( ( uint8_t )sectionRateConfig.gpioPwm, 0 );
          ledcWrite( 0, 0 );
    }

    if( sectionRateConfig.gpioDir != SectionRateConfig::Gpio::None ) {
      pinMode( ( uint8_t )sectionRateConfig.gpioDir, OUTPUT );
          ledcSetup( 1, ( uint8_t )sectionRateConfig.pwmFrequency, 8 );
          ledcAttachPin( ( uint8_t )sectionRateConfig.gpioDir, 1 );
          ledcWrite( 1, 0 );
    }

    if( sectionRateConfig.gpioEn != SectionRateConfig::Gpio::None ) {
      pinMode( ( uint8_t )sectionRateConfig.gpioEn, OUTPUT );
      digitalWrite( ( uint8_t )sectionRateConfig.gpioEn, HIGH );
    }

    switch( sectionRateConfig.rateControlType ) {
      case SectionRateConfig::RateControlType::Valve: {
        Control* labelStatusRateControlTypeHandle = ESPUI.getControl( labelStatusRateControlType );
        labelStatusRateControlTypeHandle->value = "Valve (Diverting or Blocking)";
        labelStatusRateControlTypeHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelStatusRateControlTypeHandle );

        pid.setOutputRange(-255, 255);
      }
      break;

      case SectionRateConfig::RateControlType::Motor: {
        Control* labelStatusRateControlTypeHandle = ESPUI.getControl( labelStatusRateControlType );
        labelStatusRateControlTypeHandle->value = "DC Pump Motor";
        labelStatusRateControlTypeHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelStatusRateControlTypeHandle );

        pid.setOutputRange(0, 255);
      }
      break;

      default:
        break;
    }

    switch( sectionRateConfig.outputType ) {
      case SectionRateConfig::OutputType::TB67H410: {
        Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );

        if( sectionRateConfig.gpioPwm != SectionRateConfig::Gpio::None &&
            sectionRateConfig.gpioDir != SectionRateConfig::Gpio::None &&
            sectionRateConfig.gpioEn != SectionRateConfig::Gpio::None ) {
          labelStatusOutputTypeHandle->value = "Output configured: TB67H410";
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );

          initialisation.outputType = SectionRateConfig::OutputType::TB67H410;
        } else {
          {
            labelStatusOutputTypeHandle->value = "GPIOs not correctly defined";
            labelStatusOutputTypeHandle->color = ControlColor::Carrot;
            ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
          }
        }
      }
      break;

      case SectionRateConfig::OutputType::HBridge: {
        Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );

        if( sectionRateConfig.gpioPwm != SectionRateConfig::Gpio::None &&
            sectionRateConfig.gpioDir != SectionRateConfig::Gpio::None ) {
          labelStatusOutputTypeHandle->value = "Output configured: HBridge";
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );

          initialisation.outputType = SectionRateConfig::OutputType::HBridge;
        } else {
          {
            labelStatusOutputTypeHandle->value = "GPIOs not correctly defined";
            labelStatusOutputTypeHandle->color = ControlColor::Carrot;
            ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
          }
        }
      }
      break;

      case SectionRateConfig::OutputType::IBT2: {
        Control* labelStatusOutputTypeHandle = ESPUI.getControl( labelStatusOutputType );

        if( sectionRateConfig.gpioPwm != SectionRateConfig::Gpio::None &&
            sectionRateConfig.gpioDir != SectionRateConfig::Gpio::None &&
            sectionRateConfig.gpioEn  != SectionRateConfig::Gpio::None ) {
          labelStatusOutputTypeHandle->value = "Output configured: IBT2";
          labelStatusOutputTypeHandle->color = ControlColor::Emerald;
          ESPUI.updateControlAsync( labelStatusOutputTypeHandle );

          initialisation.outputType = SectionRateConfig::OutputType::IBT2;
        } else {
          {
            labelStatusOutputTypeHandle->value = "GPIOs not correctly defined";
            labelStatusOutputTypeHandle->color = ControlColor::Carrot;
            ESPUI.updateControlAsync( labelStatusOutputTypeHandle );
          }
        }
      }
      break;

      default:
        break;

    }
  }

  if( sectionRateConfig.gpioFlowSensorEncoder != SectionRateConfig::Gpio::None ) {
    //pinMode( ( uint8_t )sectionRateConfig.gpioFlowSensorEncoder, INPUT_PULLUP );
    attachInterrupt( ( uint8_t )sectionRateConfig.gpioFlowSensorEncoder, flowSensorISR, CHANGE);
  }

  xTaskCreate( rateDisplayWorker1Hz, "rateDisplayWorker1Hz", 3096, NULL, 3, NULL );
  xTaskCreate( rateESPUIWorker1Hz, "rateESPUIWorker1Hz", 3096, NULL, 3, NULL );
  if( AOG_Control == true ) {
    xTaskCreate( rateControlWorker1Hz, "rateControlWorker1Hz", 3096, NULL, 3, NULL );
  }
}
