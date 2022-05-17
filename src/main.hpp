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

#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <AsyncUDP.h>
#include <ESPUI.h>
#include <Wire.h>

#include "jsonqueueselector.h"

extern JsonQueueSelector jsonQueueSelector;

extern uint16_t labelLoad;

extern uint16_t labelStatusOutputType;
extern uint16_t labelStatusRateControlType;
extern uint16_t labelStatusMeter;
extern uint16_t labelStatusSections;

extern bool AOG_Control;
extern SemaphoreHandle_t i2cMutex;

///////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////

struct SectionRateConfig {

  enum class Gpio : int8_t {
    Default     = -2,
    None        = -1,
    Esp32Gpio0  = 0,
    Esp32Gpio2  = 2,
    Esp32Gpio4  = 4,
    Esp32Gpio5  = 5,
    Esp32Gpio12 = 12,
    Esp32Gpio13 = 13,
    Esp32Gpio14 = 14,
    Esp32Gpio15 = 15,
    Esp32Gpio16 = 16,
    Esp32Gpio17 = 17,
    Esp32Gpio18 = 18,
    Esp32Gpio19 = 19,
    Esp32Gpio21 = 21,
    Esp32Gpio22 = 22,
    Esp32Gpio23 = 23,
    Esp32Gpio25 = 25,
    Esp32Gpio26 = 26,
    Esp32Gpio27 = 27,
    Esp32Gpio32 = 32,
    Esp32Gpio33 = 33,
    Esp32Gpio34 = 34,
    Esp32Gpio35 = 35,
    Esp32Gpio36 = 36,
    Esp32Gpio39 = 39
  };

  enum class UnitType : int8_t {
    Imperial    = 0,
    Metric = 1
  };

  char ssid[24] = "AgOpenGps";
  char password[24] = "password";
  char hostname[24] = "ESP32-Section-Rate";
  SectionRateConfig::Gpio apModePin = SectionRateConfig::Gpio::Esp32Gpio0;

  uint32_t baudrate = 115200;
  SectionRateConfig::Gpio gpioSDA = SectionRateConfig::Gpio::Default;
  SectionRateConfig::Gpio gpioSCL = SectionRateConfig::Gpio::Default;
  uint32_t i2cBusSpeed = 400000;

  SectionRateConfig::Gpio gpioAutoManualControl = SectionRateConfig::Gpio::Esp32Gpio36;
  bool invertAutoManual = false;

  bool enableOTA = false;

  //set to 1  if you want to use TB67H410 + divert valve
  //set to 2  if you want to use HBridge
  //set to 3  if you want to use IBT2
  bool invertOutput = false;
  uint16_t outputOnTime = 1.00;

  enum class OutputType : int8_t {
    None = -1,
    TB67H410 = 1,
    HBridge = 2,
    IBT2 = 3
  } outputType = OutputType::None;

  enum class RateControlType : int8_t {
    None = -1,
    Valve = 1,
    Motor = 2
  } rateControlType = RateControlType::None;

  double pwmFrequency = 1000;
  SectionRateConfig::Gpio gpioPwm = SectionRateConfig::Gpio::Esp32Gpio17;
  SectionRateConfig::Gpio gpioDir = SectionRateConfig::Gpio::Esp32Gpio16;
  SectionRateConfig::Gpio gpioEn = SectionRateConfig::Gpio::Esp32Gpio15;

  bool allowPidOverwrite = false;
  double rateControlPidKp = 20;
  double rateControlPidKi = 0.5;
  double rateControlPidKiMax = 128;
  double rateControlPidKd = 1;
  uint8_t rateControlMinPwm = 20;

  SectionRateConfig::Gpio gpioFlowSensorEncoder = SectionRateConfig::Gpio::Esp32Gpio22;
  uint16_t pulsesPerUnit = 100;
  double targetFlowRate = 10.0; // gallons per acre, liters per hectare : no conversion needed
  double flowRateOffset = 1.00;
  uint16_t tankSize = 250;

  SectionRateConfig::Gpio gpioSection1 = SectionRateConfig::Gpio::Esp32Gpio32;
  SectionRateConfig::Gpio gpioSection2 = SectionRateConfig::Gpio::Esp32Gpio33;
  SectionRateConfig::Gpio gpioSection3 = SectionRateConfig::Gpio::Esp32Gpio25;
  SectionRateConfig::Gpio gpioSection4 = SectionRateConfig::Gpio::Esp32Gpio26;
  SectionRateConfig::Gpio gpioSection5 = SectionRateConfig::Gpio::Esp32Gpio27;
  SectionRateConfig::Gpio gpioSection6 = SectionRateConfig::Gpio::Esp32Gpio14;
  SectionRateConfig::Gpio gpioSection7 = SectionRateConfig::Gpio::Esp32Gpio12;
  SectionRateConfig::Gpio gpioSection8 = SectionRateConfig::Gpio::Esp32Gpio13;

  uint16_t widthSection1 = 100;
  uint16_t widthSection2 = 0;
  uint16_t widthSection3 = 0;
  uint16_t widthSection4 = 0;
  uint16_t widthSection5 = 0;
  uint16_t widthSection6 = 0;
  uint16_t widthSection7 = 0;
  uint16_t widthSection8 = 0;

  SectionRateConfig::UnitType unitType = SectionRateConfig::UnitType::Imperial;

  uint16_t aogPortSendFrom = 5577;
  uint16_t aogPortListenTo = 8888;
  uint16_t aogPortSendTo = 9999;

  bool retainWifiSettings = true;
};
extern SectionRateConfig sectionRateConfig, sectionRateConfigDefaults;

struct Initialisation {
  SectionRateConfig::OutputType outputType = SectionRateConfig::OutputType::None;

  uint16_t portSendFrom = 5577;
  uint16_t portListenTo = 8888;
  uint16_t portSendTo = 9999;

};
extern Initialisation initialisation;


///////////////////////////////////////////////////////////////////////////
// Global Data
///////////////////////////////////////////////////////////////////////////

struct SectionSetpoints {
  bool enabled = false;
  byte sectionLowByte = 0;
  uint16_t distanceFromLine = 32020;
  uint16_t sectionsOnWidth = 0;
  String sectionState = String();
  time_t lastPacketReceived = 0;
};
extern SectionSetpoints sectionSetpoints;

struct TravelData {
  bool clearData = false; // clear TravelData every second
  double squareUnitsCovered = 0.00; //  per second
  float speed = 0;
  uint8_t counts = 0;  // averaging counts
};
extern TravelData travelData;

struct RuntimeData { // used to save current data to flash, to load for startup
  uint64_t totalFluidUnitsApplied = 0;
  uint64_t totalLandUnitsApplied = 0;
  double tankFluidRemaining = 0;
  double areaCoverage = 0.0;
  double fluidAmount = 0.0;
  uint8_t currentDisplayIndex = 0;
};
extern RuntimeData runtimeData;

///////////////////////////////////////////////////////////////////////////
// external Libraries
///////////////////////////////////////////////////////////////////////////

extern ESPUIClass ESPUI;

extern AsyncUDP udpSendFrom;

///////////////////////////////////////////////////////////////////////////
// Helper Classes
///////////////////////////////////////////////////////////////////////////
extern portMUX_TYPE mux;
class TCritSect {
    TCritSect() {
      portENTER_CRITICAL( &mux );
    }
    ~TCritSect() {
      portEXIT_CRITICAL( &mux );
    }
};

///////////////////////////////////////////////////////////////////////////
// Threads
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////////////////////////////
extern void initDOGL_Display();
extern void initWiFi();
extern void setResetButtonToRed();
extern void initESPUI();
extern void initIdleStats();
extern void initRateControl();
extern void initSectionControl();
extern void initManualSwitches();
