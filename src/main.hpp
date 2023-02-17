

#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <AsyncUDP.h>

#include <ESPUI.h>

#include <Wire.h>

#define MaxReadBuffer 100

extern uint16_t labelLoad;
extern uint16_t labelRCAppUdpData;

extern byte Temp;
extern unsigned int UnSignedTemp;
extern bool AutoOn;

extern unsigned long WifiSwitchesTimer;
extern bool WifiSwitchesEnabled;
extern byte WifiSwitches[6];

extern unsigned long CommTime[2];
extern byte ControlType[2];

extern bool DebugOn;

extern bool MasterOn[2];
extern bool IOexpanderFound;
extern byte PGNlength;

struct ModuleConfig	{
	uint8_t ID = 0;
	uint8_t IPpart2 = 168;			// ethernet IP address
	uint8_t	IPpart3 = 1;
	uint8_t IPpart4 = 200;			// 200 + ID
	uint8_t RelayOnSignal = 0;	    // value that turns on relays
	uint8_t FlowOnDirection = 0;	// sets on value for flow valve or sets motor direction
	uint8_t RelayControl = 0;		// 0 - no relays, 1 - RS485, 2 - PCA9555 8 relays, 3 - PCA9555 16 relays, 4 - MCP23017, 5 - Teensy GPIO
	uint8_t WemosSerialPort = 1;	// serial port to connect to Wemos D1 Mini
	uint8_t RelayPins[16];			// pin numbers when GPIOs are used for relay control (5)
	uint8_t LOADCELL_DOUT_PIN;	
	uint8_t LOADCELL_SCK_PIN;	
};
extern ModuleConfig MDL;

struct SensorConfig	{
	uint8_t FlowPin = 39;
  uint8_t	FwdPin = 17;
	uint8_t	RevPin = 16;
	uint8_t	PWMPin = 15;
  uint8_t	DirPin = 35; 
	bool MasterOn = false;
	bool FlowEnabled = false;
	float RateError = 0;		// rate error X 1000
	float UPM = 0;				// upm X 1000
	int16_t pwmSetting = 0;
	uint32_t CommTime = 0;
	byte InCommand = 0;			// command byte from RateController
	byte ControlType = 0;		// 0 standard, 1 combo close, 2 motor, 3 motor/weight
	uint32_t TotalPulses = 0;
	float RateSetting = 0;
	float MeterCal = 10;
	float ManualAdjust = 0;
	uint16_t ManualLast = 0;
	bool UseMultiPulses = 0;	// 0 - time for one pulse, 1 - average time for multiple pulses
	bool CalOn = false;
	byte CalPWM = 0;
};
extern SensorConfig Sensor;

struct PIDConfig {
	byte KP = 20;
	byte KI = 0;
	byte MinPWM = 50;
	byte LowMax = 100;
	byte HighMax = 255;
	byte Deadband = 3;
	byte BrakePoint = 20;
	byte AdjustTime = 0;
};
extern PIDConfig pidConfig, pidConfigDefaults;

struct AnalogConfig
{
	int16_t AIN0;	// Pressure 0
	int16_t AIN1;	// Pressure 1
	int16_t AIN2;	
	int16_t AIN3;
};
extern AnalogConfig AINs;

// Relays
extern byte RelayLo;	// sections 0-7
extern byte RelayHi;	// sections 8-15
extern byte PowerRelayLo;
extern byte PowerRelayHi;

extern uint8_t ErrorCount;
extern bool ADSfound;
extern const int16_t AdsI2Caddress;
extern uint32_t Analogtime;

extern bool ScaleFound[2];

extern float tempmonGetTemp(void);

extern int8_t WifiRSSI;
extern uint32_t WifiTime;
extern uint32_t WifiLastTime;

extern byte ESPdebug1;
extern bool ESPconnected;

// MCP23017 pins RC5, RC8
#define Relay1 0
#define Relay2 1
#define Relay3 2
#define Relay4 3
#define Relay5 4
#define Relay6 5
#define Relay7 6
#define Relay8 7

///////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////

struct SectionRateConfig {

  enum class SpeedUnits : int8_t {
    MilesPerHour      = 0,
    KilometersPerHour = 1
  } speedUnits = SpeedUnits::MilesPerHour;

  char ssid[24] = "AOG hotspot";
  char password[24] = "password";
  char hostname[24] = "Section Control";

	uint8_t gpioManualAutoSelection = 36;
	uint8_t gpioRateUp = 19;
  uint8_t gpioRateDown = 21;

	uint8_t gpioSection1 = 32;
  uint8_t gpioSection2 = 33;
  uint8_t gpioSection3 = 25;
  uint8_t gpioSection4 = 26;
  uint8_t gpioSection5 = 27;
  uint8_t gpioSection6 = 14;
  uint8_t gpioSection7 = 12;
  uint8_t gpioSection8 = 13;
  uint8_t gpioWifiLed = 34;
  uint8_t WifiLedOnLevel = HIGH;    //HIGH = LED on high, LOW = LED on low

  uint32_t baudrate = 115200;

  bool enableOTA = false;

  uint16_t rcPortSendFrom = 6100;
  uint16_t rcPortListenTo = 28888;
  uint16_t rcPortSendTo = 29999;
  uint16_t aogPortListenTo = 8888;

  bool retainWifiSettings = true;
};
extern SectionRateConfig sectionRateConfig, sectionRateConfigDefaults;


///////////////////////////////////////////////////////////////////////////
// Global Data
///////////////////////////////////////////////////////////////////////////

extern ESPUIClass ESPUI;

extern AsyncUDP udpLocalPort;
extern AsyncUDP udpRemotePort;
extern AsyncUDP udpSendFrom;
extern AsyncUDP udpSectionPort;
extern IPAddress ipDestination; //IP address to send UDP data to

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
extern void setResetButtonToRed();

extern void initESPUI();
extern void initIdleStats();
extern void initWiFi();
extern void initRateController();
extern void initRateControlUDP();
extern void initSectionUDP();
extern void initManualRate();

extern void IRAM_ATTR ISR0();
extern void GetUPM();
extern void CheckRelays();
extern void AdjustFlow();
extern void AutoControl();
extern void ManualControl();
extern void DebugTheINO();
extern void TranslateSwitchBytes();
extern void sendSwitchData();
extern void SendData();
extern void ReadPGN(uint16_t, byte, uint16_t);
extern void GetUPMflow();
extern int ControlMotor();
extern int DoPID();
extern byte CRC( byte Chk[], byte, byte );
extern byte ParseSenID( byte );
extern byte BuildModSenID( byte, byte );
extern byte ParseModID( byte );
extern bool GoodCRC( byte Chk[], byte );
