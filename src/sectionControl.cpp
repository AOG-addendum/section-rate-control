// MIT License
//
// Copyright (c) 2020 Reuben Rissler
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

#include "main.hpp"

AsyncUDP udpSectionPort;

void initSectionControl() {
  uint8_t gpioMode;
  if ( AOG_Control == false ){
    gpioMode = INPUT;
  } else {
    gpioMode = OUTPUT;
  }
  if (sectionRateConfig.gpioSection1 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection1, gpioMode);
  }
  if (sectionRateConfig.gpioSection2 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection2, gpioMode);
  }
  if (sectionRateConfig.gpioSection3 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection3, gpioMode);
  }
  if (sectionRateConfig.gpioSection4 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection4, gpioMode);
  }
  if (sectionRateConfig.gpioSection5 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection5, gpioMode);
  }
  if (sectionRateConfig.gpioSection6 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection6, gpioMode);
  }
  if (sectionRateConfig.gpioSection7 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection7, gpioMode);
  }
  if (sectionRateConfig.gpioSection8 != SectionRateConfig::Gpio::None ){
    pinMode ( ( uint8_t )sectionRateConfig.gpioSection8, gpioMode);
  }

  if ( udpSectionPort.listen( initialisation.portListenTo ) ) {
    udpSectionPort.onPacket( []( AsyncUDPPacket packet ) {
      uint8_t* data = packet.data();
      if ( data[1] + ( data[0] << 8 ) != 0x8081 ) {
        return;
      }
      uint16_t pgn = data[3] + ( data[2] << 8 );
      if ( pgn == 0x7FFE ) {
        travelData.speed = ( float )( (data[5] | data[6] << 8)) ; // * 0.1 for true speed
        sectionSetpoints.lastPacketReceived = millis();
        sectionSetpoints.sectionsOnWidth = 0;
        sectionSetpoints.sectionLowByte = data[11];
        if( sectionSetpoints.sectionLowByte == 0 || travelData.speed == 0 ){
          sectionSetpoints.enabled = false;
        } else {
          sectionSetpoints.enabled = true;
        }
        if (sectionRateConfig.gpioSection1 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 0);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection1, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection1;
          }
          sectionSetpoints.sectionState = String(state);
        }
        if (sectionRateConfig.gpioSection2 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 1);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection2, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection2;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection3 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 2);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection3, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection3;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection4 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 3);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection4, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection4;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection5 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 4);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection5, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection5;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection6 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 5);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection6, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection6;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection7 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 6);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection7, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection7;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if (sectionRateConfig.gpioSection8 != SectionRateConfig::Gpio::None ){
          bool state = bitRead(sectionSetpoints.sectionLowByte, 7);
          digitalWrite ( ( uint8_t )sectionRateConfig.gpioSection8, state);
          if( state ){
            sectionSetpoints.sectionsOnWidth += sectionRateConfig.widthSection8;
          }
          sectionSetpoints.sectionState += String(state);
        }
        if( ( SectionRateConfig::UnitType )sectionRateConfig.unitType == SectionRateConfig::UnitType::Imperial ) {
          travelData.speed *= 62.13; // * 0.001 for true MPH
          if( travelData.clearData == true ){
            travelData.squareUnitsCovered = (( travelData.speed * 1.4666 ) * sectionSetpoints.sectionsOnWidth ) / 12; // MPH * 1.4666 = feet per second
            travelData.clearData = false;
            travelData.counts = 1;
          } else {
            travelData.squareUnitsCovered += (( travelData.speed * 1.4666 ) * sectionSetpoints.sectionsOnWidth ) / 12;
            travelData.counts += 1;
          }
        } else {
          travelData.speed *= 100; // * 0.001 for true KMH
          if( travelData.clearData == true ){
            travelData.squareUnitsCovered = (( travelData.speed * 0.2778 ) * sectionSetpoints.sectionsOnWidth ) / 10; // KMH * 0.2778 = meters per second
            travelData.clearData = false;
            travelData.counts = 1;
          } else {
            travelData.squareUnitsCovered += (( travelData.speed * 0.2778 ) * sectionSetpoints.sectionsOnWidth ) / 10;
            travelData.counts += 1;
          }
        }
        Control* labelStatusSectionsHandle = ESPUI.getControl( labelStatusSections );
        String str;
        str.reserve( 30 );
        str = "Control: ";
        str += AOG_Control ? "AOG" : "Manual";
        str += ", AOG Section State (1...8): ";
        str += sectionSetpoints.sectionState;
        if( ( SectionRateConfig::UnitType )sectionRateConfig.unitType == SectionRateConfig::UnitType::Imperial ) {
          str += ", Feet: ";
          str += ( double )( ( double ) sectionSetpoints.sectionsOnWidth / 12 );
        } else {
          str += ", Meters: ";
          str += ( double )( ( double ) sectionSetpoints.sectionsOnWidth / 10 );
        }
        labelStatusSectionsHandle->value = str;
        labelStatusSectionsHandle->color = ControlColor::Emerald;
        ESPUI.updateControlAsync( labelStatusSectionsHandle );
      }
    } );
  }
}
