
#include "main.hpp"
#include "jsonFunctions.hpp"

#include <ESPUI.h>

uint16_t buttonReset;

void setResetButtonToRed() {
  ESPUI.getControl( buttonReset )->color = ControlColor::Alizarin;
  ESPUI.updateControlAsync( buttonReset );
}

void saveConfigToSPIFFS() {
}

void addGpioOutput( uint16_t parent ) {
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 0", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio0 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 2", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio2 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 4", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio4 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 5", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio5 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 12", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio12 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 13 / A12", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio13 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 14", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio14 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 15", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio15 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 16", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio16 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 17", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio17 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 18", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio18 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 19", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio19 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 21", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio21 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 22", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio22 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 23", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio23 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 25", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio25 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 26", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio26 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 27", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio27 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 32", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio32 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 33", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio33 ), ControlColor::Alizarin, parent );
}
void addGpioInput( uint16_t parent ) {
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 34 (input only)", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio34 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 35 (input only)", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio35 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 36 (input only)", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio36 ), ControlColor::Alizarin, parent );
  ESPUI.addControl( ControlType::Option, "ESP32 GPIO 39 (input only)", String( ( uint8_t )SectionRateConfig::Gpio::Esp32Gpio39 ), ControlColor::Alizarin, parent );
}

void initESPUI ( void ) {

      labelLoad = ESPUI.addControl( ControlType::Label, "Load:", "", ControlColor::Turquoise );
      labelStatusRateControlType = ESPUI.addControl( ControlType::Label, "Rate Control Type:", "No Type selected", ControlColor::Turquoise );

      buttonReset = ESPUI.addControl( ControlType::Button, "Store the Settings", "Apply", ControlColor::Emerald, Control::noParent,
      []( Control * control, int id ) {
        if( id == B_UP ) {
          saveConfig();
        }
      } );

      buttonReset = ESPUI.addControl( ControlType::Button, "If this turns red, you have to", "Apply & Reboot", ControlColor::Emerald, Control::noParent,
      []( Control * control, int id ) {
        if( id == B_UP ) {
          saveConfig();
          SPIFFS.end();
          ESP.restart();
        }
      } );

      uint16_t tabConfigurations;

      // Status Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Status", "Status" );

        labelStatusOutputType = ESPUI.addControl( ControlType::Label, "Output Type:", "No Output configured", ControlColor::Turquoise, tab );
        labelStatusMeter = ESPUI.addControl( ControlType::Label, "Rate Meter Input:", "No Input configured", ControlColor::Turquoise, tab );
        labelStatusSectionsOnWidth = ESPUI.addControl( ControlType::Label, "Sections 'On' Width:", "No data from AOG", ControlColor::Turquoise, tab );

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

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Pin to show AP mode*", String( ( int )sectionRateConfig.apModePin ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.apModePin = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }

        ESPUI.addControl( ControlType::Switcher, "OTA Enabled*", sectionRateConfig.enableOTA ? "1" : "0", ControlColor::Wetasphalt, tab,
        []( Control * control, int id ) {
          sectionRateConfig.enableOTA = control->value.toInt() == 1;
          setResetButtonToRed();
        } );

          ESPUI.addControl( ControlType::Number, "Port to send from*", String( sectionRateConfig.aogPortSendFrom ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.aogPortSendFrom = control->value.toInt();
            setResetButtonToRed();
          } );

          ESPUI.addControl( ControlType::Number, "Port to send to*", String( sectionRateConfig.aogPortSendTo ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.aogPortSendTo = control->value.toInt();
            setResetButtonToRed();
          } );

          ESPUI.addControl( ControlType::Number, "Port to listen to*", String( sectionRateConfig.aogPortListenTo ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.aogPortListenTo = control->value.toInt();
            setResetButtonToRed();
          } );

      }

      // Flow Meter Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Flow Meter", "Flow Meter" );

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Unit Type*", String( ( int )sectionRateConfig.unitType ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.unitType = ( SectionRateConfig::UnitType )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "Imperial", "0", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "Metric", "1", ControlColor::Alizarin, sel );
        }

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Flow Sensor Encoder GPIO*", String( ( int )sectionRateConfig.gpioFlowSensorEncoder ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioFlowSensorEncoder = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioInput( sel );
          addGpioOutput( sel );
        }

        if( ( SectionRateConfig::UnitType )sectionRateConfig.unitType == SectionRateConfig::UnitType::Imperial ) {
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Pulses per Gallon", String( ( int )sectionRateConfig.pulsesPerUnit ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.pulsesPerUnit = control->value.toInt();
            } );
            ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "4000", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Gallons per Acre (target)", String( ( int )sectionRateConfig.targetFlowRate ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.targetFlowRate = control->value.toDouble();
          } );
            ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Gallons per Acre Offset Multiplier", String( ( int )sectionRateConfig.flowRateOffset ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.flowRateOffset = control->value.toDouble();
          } );
            ESPUI.addControl( ControlType::Min, "Min", "-100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
        } else {
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Pulses per Liter", String( ( int )sectionRateConfig.pulsesPerUnit ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.pulsesPerUnit = control->value.toInt();
          } );
            ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "4000", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Liters per Hectare (target)", String( ( float )sectionRateConfig.targetFlowRate ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.targetFlowRate = control->value.toDouble();
          } );
            ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
          {
            uint16_t num = ESPUI.addControl( ControlType::Number, "Liters per Hectare Offset Multiplier", String( ( int )sectionRateConfig.flowRateOffset ), ControlColor::Peterriver, tab,
            []( Control * control, int id ) {
              sectionRateConfig.flowRateOffset = control->value.toDouble();
          } );
            ESPUI.addControl( ControlType::Min, "Min", "-100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Max, "Max", "100", ControlColor::Peterriver, num );
            ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
          }
        }

        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Tank Size", String( ( int )sectionRateConfig.tankSize ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.tankSize = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }

      }

      // Rate Control Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Rate Control", "Rate Control" );

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Output Type*", String( ( int )sectionRateConfig.outputType ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.outputType = ( SectionRateConfig::OutputType )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "TB67H410", "1", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "HBridge", "2", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "IBT2", "3", ControlColor::Alizarin, sel );
        }

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Rate Control Type*", String( ( int )sectionRateConfig.rateControlType ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlType = ( SectionRateConfig::RateControlType )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "Valve (Diverting or Blocking)", "1", ControlColor::Alizarin, sel );
          ESPUI.addControl( ControlType::Option, "DC Motor Pump", "2", ControlColor::Alizarin, sel );
        }

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Output Pin for PWM (or forward)*", String( ( int )sectionRateConfig.gpioPwm ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioPwm = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Output Pin for Dir (or reverse)*", String( ( int )sectionRateConfig.gpioDir ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioDir = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "Output Pin for Enable*", String( ( int )sectionRateConfig.gpioEn ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioEn = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }

        ESPUI.addControl( ControlType::Switcher, "Invert Output", sectionRateConfig.invertOutput ? "1" : "0", ControlColor::Peterriver, tab,
        []( Control * control, int id ) {
          sectionRateConfig.invertOutput = control->value.toInt() == 1;
        } );

        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PWM Frequency*", String( sectionRateConfig.pwmFrequency, 2 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.pwmFrequency = control->value.toDouble();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "4000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
      }

      // Rate Control PID Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Rate Control PID", "Rate Control PID" );

        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PID Kp", String( sectionRateConfig.rateControlPidKp, 4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlPidKp = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "50", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "0.1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PID Ki", String( sectionRateConfig.rateControlPidKi, 4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlPidKi = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "50", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "0.01", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PID Ki Max", String( sectionRateConfig.rateControlPidKiMax, 2 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlPidKiMax = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "255", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1.00", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PID Kd", String( sectionRateConfig.rateControlPidKd, 4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlPidKd = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "50", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "0.01", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "PID Min PWM", String( sectionRateConfig.rateControlPidMinPwm, 4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.rateControlPidMinPwm = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "50", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "0.01", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Output On Millis (* PWM)", String( sectionRateConfig.outputOnTime, 4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.outputOnTime = control->value.toDouble();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "100", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "0.01", ControlColor::Peterriver, num );
        }
      }

      // Section GPIO Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Section GPIO", "Section GPIO" );

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 1*", String( ( int )sectionRateConfig.gpioSection1 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection1 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 2*", String( ( int )sectionRateConfig.gpioSection2 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection2 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 3*", String( ( int )sectionRateConfig.gpioSection3 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection3 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 4*", String( ( int )sectionRateConfig.gpioSection4 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection4 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 5*", String( ( int )sectionRateConfig.gpioSection5 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection5 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 6*", String( ( int )sectionRateConfig.gpioSection6 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection6 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 7*", String( ( int )sectionRateConfig.gpioSection7 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection7 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "IO Pin for Section 8*", String( ( int )sectionRateConfig.gpioSection8 ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioSection8 = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
        }
      }

      // Section Width Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Section Width", "Section Width" );

        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 1", String( ( int )sectionRateConfig.widthSection1 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection1 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 2", String( ( int )sectionRateConfig.widthSection2 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection2 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 3", String( ( int )sectionRateConfig.widthSection3 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection3 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 4", String( ( int )sectionRateConfig.widthSection4 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection4 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 5", String( ( int )sectionRateConfig.widthSection5 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection5 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 6", String( ( int )sectionRateConfig.widthSection6 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection6 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 7", String( ( int )sectionRateConfig.widthSection7 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection7 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }
        {
          uint16_t num = ESPUI.addControl( ControlType::Number, "Section 8", String( ( int )sectionRateConfig.widthSection8 ), ControlColor::Peterriver, tab,
          []( Control * control, int id ) {
            sectionRateConfig.widthSection8 = control->value.toInt();
          } );
          ESPUI.addControl( ControlType::Min, "Min", "0", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Max, "Max", "2000", ControlColor::Peterriver, num );
          ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, num );
        }

      }

      // Other Tab
      {
        uint16_t tab = ESPUI.addControl( ControlType::Tab, "Other", "Other" );
        ESPUI.addControl( ControlType::Label, "Attention:", "These Buttons here reset the whole config. This affects the WIFI too, if not configured otherwise below. You have to press \"Apply & Reboot\" above to actualy store them.", ControlColor::Carrot, tab );

        ESPUI.addControl( ControlType::Label, "OTA Update:", "<a href='/update'>Update</a>", ControlColor::Carrot, tab );

        ESPUI.addControl( ControlType::Label, "Download the config:", "<a href='config.json'>Configuration</a>", ControlColor::Carrot, tab );

        ESPUI.addControl( ControlType::Label, "Upload the config:", "<form method='POST' action='/upload-config' enctype='multipart/form-data'><input name='f' type='file'><input type='submit'></form>", ControlColor::Carrot, tab );

        {
          uint16_t sel = ESPUI.addControl( ControlType::Select, "GPIO Pin for Auto / Manual*", String( ( int )sectionRateConfig.gpioAutoManualControl ), ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.gpioAutoManualControl = ( SectionRateConfig::Gpio )control->value.toInt();
            setResetButtonToRed();
          } );
          ESPUI.addControl( ControlType::Option, "None", "-1", ControlColor::Alizarin, sel );
          addGpioOutput( sel );
          addGpioInput( sel );
        }
        {
          ESPUI.addControl( ControlType::Switcher, "Invert Auto / Manual*", sectionRateConfig.invertAutoManual ? "1" : "0", ControlColor::Wetasphalt, tab,
          []( Control * control, int id ) {
            sectionRateConfig.invertAutoManual = control->value.toInt() == 1;
            setResetButtonToRed();
          } );
        }
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

      /*
      * .begin loads and serves all files from PROGMEM directly.
      * If you want to serve the files from SPIFFS use ESPUI.beginSPIFFS
      * (.prepareFileSystem has to be run in an empty sketch before)
      */

      /*
      * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
      * SECURE way of limiting access.
      * Anyone who is able to sniff traffic will be able to intercept your password
      * since it is transmitted in cleartext. Just add a username and password,
      * for example begin("ESPUI Control", "username", "password")
      */
      static String title;
      title = "AOG Section Control :: ";
      title += sectionRateConfig.hostname;

      ESPUI.begin( title.c_str() );

      ESPUI.server->on( "/config.json", HTTP_GET, []( AsyncWebServerRequest * request ) {
        request->send( SPIFFS, "/config.json", "application/json", true );
      } );
      ESPUI.server->on( "/calibration.json", HTTP_GET, []( AsyncWebServerRequest * request ) {
        request->send( SPIFFS, "/calibration.json", "application/json", true );
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

      // upload a file to /upload-calibration
      ESPUI.server->on( "/upload-calibration", HTTP_POST, []( AsyncWebServerRequest * request ) {
        request->send( 200 );
      }, [tabConfigurations]( AsyncWebServerRequest * request, String filename, size_t index, uint8_t* data, size_t len, bool final ) {
        if( !index ) {
          request->_tempFile = SPIFFS.open( "/calibration.json", "w" );
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
