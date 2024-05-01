

#include "main.hpp"
#include "jsonFunctions.hpp"

AsyncUDP udpSectionPort;
SectionConfig sectionConfig;

void initAutoSectionUDP(){

  if( udpSectionPort.listen( sectionRateConfig.aogPortListenTo )){
    udpSectionPort.onPacket([](AsyncUDPPacket packet){
      uint8_t* data = packet.data();
      if ( data[1] + ( data[0] << 8 ) != 0x8081 ) {
        return;
      }
			uint8_t len = packet.length();
			uint16_t pgn = data[3] + ( data[2] << 8 );
			if( pgn == 32766 ){ // section control
				sectionsOn = data[11];
				sectionsUpdateMillis = millis();
        digitalWrite( sectionConfig.rowOne, bitRead( sectionsOn, 1 ));
			}
		});
	}
}