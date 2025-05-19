

#include "main.hpp"
#include "jsonFunctions.hpp"

AsyncUDP udpSendFrom;
AsyncUDP udpLocalPort;
AsyncUDP udpRemotePort;

byte DataOut[50];

uint32_t TestWeight = 430000;
uint8_t PGNlength;

uint8_t sectionsOn = 0;
unsigned long sectionsUpdateMillis;

uint8_t loopCounterUDP;

void SendData(){

	//PGN32400, Rate info from module to RC
	//0     HeaderLo    144
	//1     HeaderHi    126
	//2     Mod/Sen ID          0-15/0-15
	//3	    rate applied Lo 	1000 X actual
	//4     rate applied Mid
	//5	    rate applied Hi
	//6	    acc.Quantity Lo		10 X actual
	//7	    acc.Quantity Mid
	//8     acc.Quantity Hi
	//9     PWM Lo
	//10    PWM Hi
	//11    Status
	//      bit 0   sensor connected
	//12    CRC
	DataOut[0] = 144;
	DataOut[1] = 126;
	DataOut[2] = BuildModSenID(MDL.ID, 0);

	// rate applied, 1000 X actual
	uint32_t Applied = Sensor.UPM * 1000;
	DataOut[3] = Applied;
	DataOut[4] = Applied >> 8;
	DataOut[5] = Applied >> 16;

	// accumulated quantity, 10 X actual
	if( Sensor.MeterCal > 0 ){
		long Units = ( Sensor.TotalPulses * 10.0 ) / Sensor.MeterCal;
		DataOut[6] = Units;
		DataOut[7] = Units >> 8;
		DataOut[8] = Units >> 16;
	}
	else{
		DataOut[6] = 0;
		DataOut[7] = 0;
		DataOut[8] = 0;
	}
	DataOut[9] = Sensor.pwmSetting;
	DataOut[10] = Sensor.pwmSetting >> 8;
	// status
	DataOut[11] = 0;
	if (millis()-Sensor.RateCommTime < 4000) DataOut[11] |= 0b00000001;
	DataOut[12] = CRC(DataOut, 12, 0);
	udpSendFrom.writeTo( DataOut, 13, ipDestination, sectionRateConfig.rcPortSendTo );

	//PGN32401, module info from module to RC
	//0     145
	//1     126
	//2     module ID
	//3     Pressure Lo X 10
	//4     Pressure Hi
	//5     -
	//6     -
	//7     -
	//8     -
	//9     -
	//10    -
	//11    InoID lo
	//12    InoID hi
	//13    status
	//      bit 0   work switch
	//      bit 1   wifi rssi < -80
	//      bit 2	wifi rssi < -70
	//      bit 3	wifi rssi < -65
	//      bit 4   ethernet connected
	//      bit 5   good pin configuration
	//14    CRC

	DataOut[0] = 145;
	DataOut[1] = 126;
	DataOut[2] = MDL.ID;
	DataOut[3] = (byte) 0; //CurrentPressure;
	DataOut[4] = (byte) 0; //(CurrentPressure >> 8);
	DataOut[5] = 0;
	DataOut[6] = 0;
	DataOut[7] = 0;
	DataOut[8] = 0;
	DataOut[9] = 0;
	DataOut[10] = 0;
	DataOut[11] = (byte)3124;
	DataOut[12] = (byte)(3124 >> 8);

	// status
	DataOut[13] = 0b00000001;
	//if ( true ) DataOut[13] |= 0b00000001;

	int8_t WifiStrength = WiFi.RSSI();
	if (WifiStrength < -80){
		DataOut[13] |= 0b00000010;
	}
	else if (WifiStrength < -70){
		DataOut[13] |= 0b00000100;
	}
	else{
		DataOut[13] |= 0b00001000;
	}
	DataOut[13] |= 0b00100000; // good pins
	DataOut[14] = CRC(DataOut, 14, 0);
	udpSendFrom.writeTo( DataOut, 15, ipDestination, sectionRateConfig.rcPortSendTo );
	sendSwitchData();

	if( ++loopCounterUDP >= 12 ){ // 2.5 seconds
		loopCounterUDP = 0;
		// 0    127
		// 1    200
		uint8_t helloFromMachine[] = { 128, 129, 123, 123, 5, 0, 0, 0, 0, 0, 71 };
		helloFromMachine[5] = 0; // relayLo
		helloFromMachine[6] = 0; // relayHi
		udpSendFrom.writeTo( helloFromMachine, sizeof( helloFromMachine ), ipDestination, sectionRateConfig.aogPortSendTo );
	}
}

void initAutoRateControlUDP(){
  if( udpLocalPort.listen( sectionRateConfig.rcPortListenTo )){
    udpLocalPort.onPacket([](AsyncUDPPacket packet){
      uint8_t* Data = packet.data();
      uint16_t PGN = Data[0] + ( Data[1] << 8 );
			uint8_t len = packet.length();
			switch (PGN){

				case 32500:
					//PGN32500, Rate settings from RC to module
					//0	    HeaderLo		    244
					//1	    HeaderHi		    126
					//2     Mod/Sen ID          0-15/0-15
					//3	    rate set Lo		    1000 X actual
					//4     rate set Mid
					//5	    rate set Hi
					//6	    Flow Cal Lo	        1000 X actual
					//7     Flow Cal Mid
					//8     Flow Cal Hi
					//9	    Command
					//	        - bit 0		    reset acc.Quantity
					//	        - bit 1,2,3		control type 0-4
					//	        - bit 4		    MasterOn
					//          - bit 5         -
					//          - bit 6         AutoOn
					//          - bit 7         -
					//10    manual pwm Lo
					//11    manual pwm Hi
					//12    -
					//13    CRC

					PGNlength = 14;
					if (len > PGNlength - 1){
						if (GoodCRC(Data, PGNlength)){
							byte tmp = Data[2];
							if (ParseModID(tmp) == MDL.ID){
								byte ID = ParseSenID(tmp);  // sensor ID
								if (ID == MDL.ID){

									// rate setting, 1000 times actual
									uint32_t RateSet = Data[3] | (uint32_t)Data[4] << 8 | (uint32_t)Data[5] << 16;
									Sensor.TargetUPM = (float)(RateSet * 0.001);

									// Meter Cal, 1000 times actual
									uint32_t Temp = Data[6] | (uint32_t)Data[7] << 8 | (uint32_t)Data[8] << 16;
									Sensor.MeterCal = Temp * 0.001;

									// command byte
									Sensor.InCommand = Data[9];
									if ((Sensor.InCommand & 1) == 1) Sensor.TotalPulses = 0; // reset accumulated count

									Sensor.ControlType = 0;
									if ((Sensor.InCommand & 2) == 2) Sensor.ControlType += 1;
									if ((Sensor.InCommand & 4) == 4) Sensor.ControlType += 2;
									if ((Sensor.InCommand & 8) == 8) Sensor.ControlType += 4;

									Sensor.MasterOn = ((Sensor.InCommand & 16) == 16);

									Sensor.AutoOn = ((Sensor.InCommand & 64) == 64);

									int16_t tmp = Data[10] | Data[11] << 8;
                  					Sensor.ManualAdjust = tmp;

									Sensor.RateCommTime = millis();
								}
							}
						}
					}
					break;

				case 32502:
					// PGN32502, PID from RC to module
					// 0    246
					// 1    126
					// 2    Mod/Sen ID     0-15/0-15
					// 3    KP
					// 4    KI
					// 5    KD
					// 6    MinPWM
					// 7    MaxPWM
					// 8    PID scaling
					// 9    CRC
        			PGNlength = 10;
					if (len > PGNlength - 1){
						if (GoodCRC(Data, PGNlength)){
							byte tmp = Data[2];
							if (ParseModID(tmp) == MDL.ID){
								byte ID = ParseSenID(tmp);
								if (ID == MDL.ID){
                  					double PIDscale = pow(10, Data[8] * -1);

									pidConfig.KP = (double)(Data[3] * PIDscale);
									pidConfig.KI = (double)(Data[4] * PIDscale);
									pidConfig.KD = (double)(Data[5] * PIDscale);
									pidConfig.MinPWM = (double)(Data[6] * PIDscale);
									pidConfig.MaxPWM = (double)(Data[7] * PIDscale);
									Sensor.PIDCommTime = millis();
								}
								//savePIDConfig();
							}
						}
					}
					break;
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

