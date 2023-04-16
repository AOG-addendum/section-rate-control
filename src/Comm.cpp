

#include "main.hpp"
#include "jsonFunctions.hpp"

AsyncUDP udpSendFrom;
AsyncUDP udpLocalPort;
AsyncUDP udpRemotePort;
AsyncUDP udpSectionPort;

byte DataOut[50];

const byte PGN32500Length = 31;
const byte PGN32501Length = 8;
const byte PGN32502Length = 7;
const byte PGN32503Length = 6;
const byte PGN32613Length = 13;
const byte PGN32614Length = 16;
const byte PGN32616Length = 12;
const byte PGN32619Length = 6;
const byte PGN32621Length = 12;

uint32_t TestWeight = 430000;

uint8_t sectionsOn = 0;
unsigned long sectionsUpdateMillis;

void SendData(){

	//PGN32613 to Rate Controller from Arduino
	//0	HeaderLo		101
	//1	HeaderHi		127
	//2 Mod/Sen ID      0-15/0-15
	//3	rate applied Lo 	10 X actual
	//4 rate applied Mid
	//5	rate applied Hi
	//6	acc.Quantity Lo		10 X actual
	//7	acc.Quantity Mid
	//8	acc.Quantity Hi
	//9 PWM Lo
	//10 PWM Hi
	//11 Status
	//12 crc
	DataOut[0] = 101;
	DataOut[1] = 127;
	DataOut[2] = BuildModSenID(MDL.ID, 0);
	// rate applied, 10 X actual
	DataOut[3] = Sensor.UPM * 10;
	DataOut[4] = (int)(Sensor.UPM * 10) >> 8;
	DataOut[5] = (int)(Sensor.UPM * 10) >> 16;
	// accumulated quantity, 10 X actual
	if( Sensor.MeterCal > 0 ){
		long Units = Sensor.TotalPulses * 10.0 / Sensor.MeterCal;
		DataOut[6] = Units;
		DataOut[7] = Units >> 8;
		DataOut[8] = Units >> 16;
	}
	else{
		DataOut[6] = 0;
		DataOut[7] = 0;
		DataOut[8] = 0;
	}
	DataOut[9] = Sensor.pwmSetting * 10;
	DataOut[10] = (Sensor.pwmSetting * 10) >> 8;
	// status
	// bit 0    - sensor 0 receiving rate controller data
	// bit 1    - sensor 1 receiving rate controller data
	// bit 2    - wifi rssi < -80
	// bit 3	- wifi rssi < -70
	// bit 4	- wifi rssi < -65
	DataOut[11] = 0;
	if (millis()-Sensor.CommTime < 4000) DataOut[11] |= 0b00000001;
	if (millis()-Sensor.CommTime < 4000) DataOut[11] |= 0b00000010;
	DataOut[12] = CRC(DataOut, PGN32613Length - 1, 0);
	udpSendFrom.writeTo( DataOut, 13, ipDestination, sectionRateConfig.rcPortSendTo );

	//PGN 32621, pressures to RC
	//0    109
	//1    127
	//2    module ID
	//3    sensor 0, Lo
	//4    sensor 0, Hi
	//5    sensor 1, Lo
	//6    sensor 1, Hi
	//7    sensor 2, Lo
	//8    sensor 2, Hi
	//9    sensor 3, Lo
	//10   sensor 3, Hi
	//11   CRC
	DataOut[0] = 109;
	DataOut[1] = 127;
	DataOut[2] = MDL.ID;
	DataOut[3] = (byte)AINs.AIN0;
	DataOut[4] = (byte)(AINs.AIN0 >> 8);
	DataOut[5] = (byte)AINs.AIN1;
	DataOut[6] = (byte)(AINs.AIN1 >> 8);
	DataOut[7] = (byte)AINs.AIN2;
	DataOut[8] = (byte)(AINs.AIN2 >> 8);
	DataOut[9] = (byte)AINs.AIN3;
	DataOut[10] = (byte)(AINs.AIN3 >> 8);
	DataOut[11] = CRC(DataOut, PGN32621Length - 1, 0);
	udpSendFrom.writeTo( DataOut, 12, ipDestination, sectionRateConfig.rcPortSendTo );

	sendSwitchData();

}

void initAutoRateControlUDP(){
  if( udpLocalPort.listen( sectionRateConfig.rcPortListenTo )){
    udpLocalPort.onPacket([](AsyncUDPPacket packet){
      uint8_t* Data = packet.data();
      uint16_t PGN = Data[0] + ( Data[1] << 8 );
			uint8_t len = packet.length();
			switch (PGN){

				case 32614:
					//PGN32614 to Arduino from Rate Controller, 16 bytes
					//0	HeaderLo		102
					//1	HeaderHi		127
					//2 Controller ID
					//3	relay Lo		0 - 7
					//4	relay Hi		8 - 15
					//5	rate set Lo		10 X actual
					//6 rate set Mid
					//7	rate set Hi		10 X actual
					//8	Flow Cal Lo		1000 X actual
					//9	Flow Cal Mid
					//10 Flow Cal Hi
					//11	Command
					//- bit 0		    reset acc.Quantity
					//- bit 1, 2		valve type 0 - 3
					//- bit 3		    MasterOn
					//- bit 4           0 - average time for multiple pulses, 1 - time for one pulse
					//- bit 5           AutoOn
					//- bit 6           Debug pgn on
					//- bit 7           Calibration on
					//12    power relay Lo      list of power type relays 0-7
					//13    power relay Hi      list of power type relays 8-15
					//14    Cal PWM     calibration pwm
					//15    crc
					if (len > PGN32614Length - 1){
						if (GoodCRC(Data, PGN32614Length)){
							byte tmp = Data[2];
							if (ParseModID(tmp) == MDL.ID){
								byte ID = ParseSenID(tmp);  // sensor ID
								if (ID == MDL.ID){
									RelayLo = Data[3];
									RelayHi = Data[4];

									// command byte
									Sensor.InCommand = Data[11];
									if ((Sensor.InCommand & 1) == 1) Sensor.TotalPulses = 0; // reset accumulated count

									Sensor.ControlType = 0;
									if ((Sensor.InCommand & 2) == 2) Sensor.ControlType += 1;
									if ((Sensor.InCommand & 4) == 4) Sensor.ControlType += 2;

									Sensor.MasterOn = ((Sensor.InCommand & 8) == 8);
									Sensor.UseMultiPulses = ((Sensor.InCommand & 16) == 16);
									AutoOn = ((Sensor.InCommand & 32) == 32);
									Sensor.CalOn = ((Sensor.InCommand & 128) == 128);

									// rate setting, 10 times actual
									int RateSet = Data[5] | Data[6] << 8 | Data[7] << 16;
									if (AutoOn){
										Sensor.RateSetting = (float)RateSet * 0.1;
									}
									else{
										Sensor.ManualAdjust = (float)RateSet * 0.1;
									}

									// Meter Cal, 1000 X actual
									uint32_t Temp = Data[8] | Data[9] << 8 | Data[10] << 16;
									Sensor.MeterCal = (float)Temp * 0.001;

									// power relays
									PowerRelayLo = Data[12];
									PowerRelayHi = Data[13];

									// cal
									Sensor.CalPWM = Data[14];

									Sensor.CommTime = millis();
								}
							}
						}
					}
					break;

				case 32616:
					// PID to Arduino from RateController, 12 bytes

					if (len > PGN32616Length - 1){
						if (GoodCRC(Data, PGN32616Length)){
							byte tmp = Data[2];
							if (ParseModID(tmp) == MDL.ID){
								byte ID = ParseSenID(tmp);
								if (ID == MDL.ID){
									pidConfig.KP = Data[3];
									pidConfig.MinPWM = Data[4];
									pidConfig.LowMax = Data[5];
									pidConfig.HighMax = Data[6];
									pidConfig.Deadband = Data[7];
									pidConfig.BrakePoint = Data[8];
									pidConfig.AdjustTime = Data[9];
									pidConfig.KI = Data[10];
								}
								savePIDConfig();
							}
						}
					}
					break;

				case 32619:
					// from Wemos D1 mini, 6 bytes
					// section buttons

					if (len > PGN32619Length - 1){
						if (GoodCRC(Data, PGN32619Length)){
							for (int i = 2; i < 6; i++){
								WifiSwitches[i] = Data[i];
							}
							WifiSwitchesEnabled = true;
							WifiSwitchesTimer = millis();
						}
					}
				break;
				}
		});
	}
}

void initSectionUDP(){

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
			if( pgn == 32766 ){ // section control
				sectionsOn = data[11];
				sectionsUpdateMillis = millis();
				Wire.beginTransmission( 0x20 );
				Wire.write( 0x12 ); // address port A
				Wire.write( sectionsOn );  // value to send
				Wire.endTransmission();
			}
		});
	}
}

void sendSwitchData (){
	DataOut[0] = 106;
	DataOut[1] = 127;
	// read switches
	DataOut[2] = 1;
	//DataOut[2] | ( 1 << 0 ); //Auto
	DataOut[2] | ( 1 << 1 ); //MasterOn
	//bit[2] = false; //MasterOff
	//bit[3] = false; //RateUp
	//bit[4] = false; //RateDown
	DataOut[3] = 255;
	DataOut[4] = 255;
	DataOut[5] = CRC(DataOut, 6, 0);
	udpSendFrom.writeTo( DataOut, 6, ipDestination, sectionRateConfig.rcPortSendTo );

}

