

#include "main.hpp"
#include "jsonFunctions.hpp"

AsyncUDP udpSectionPort;

void initAutoSectionUDP(){

  Wire.beginTransmission( 0x20 );
  Wire.write( 0x00 ); // IODIRA register
  Wire.write( 0x00 ); // set entire PORT A to output
  Wire.endTransmission();
  if( udpSectionPort.listen( sectionRateConfig.aogPortListenTo )){
    udpSectionPort.onPacket([](AsyncUDPPacket packet){
      uint8_t* data = packet.data();
      if ( data[1] + ( data[0] << 8 ) != 0x8081 ) {
        return;
      }
			uint8_t len = packet.length();
			uint16_t pgn = data[3] + ( data[2] << 8 );
      switch( pgn ){
			  case 32766: { // section control
          sectionsOn = data[11];
          sectionsUpdateMillis = millis();
          Wire.beginTransmission( 0x20 );
          Wire.write( 0x12 ); // address port A
          Wire.write( sectionsOn );  // value to send
          Wire.endTransmission();
        }
        break;

        case 32712: {
          // PGN32712, Hello from AgIO to module
          // 0    127
          // 1    200
          uint8_t helloFromMachine[] = { 128, 129, 123, 123, 5, 0, 0, 0, 0, 0, 71 };
          helloFromMachine[5] = 0; // relayLo
          helloFromMachine[6] = 0; // relayHi
          udpSendFrom.broadcastTo( helloFromMachine, sizeof( helloFromMachine ), sectionRateConfig.aogPortSendTo );
        }
        break;

        default:
          break;
      }
		});
	}
}

void manualSection10Hz ( void* z ) {
  constexpr TickType_t xFrequency = 100;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for( ;; ) {
    Wire.beginTransmission(0x20);
    Wire.write(0x13); // address PORT B
    Wire.endTransmission();
    Wire.requestFrom(0x20, 1); // request one byte of data
    sectionsOn = Wire.read(); // store incoming byte into "sectionsOn"
    sectionsUpdateMillis = millis();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initManualSection (){
  xTaskCreate( manualSection10Hz, "manualSection", 4096, NULL, 5, NULL );
}