/* This grabs the data sent from the Arduino and relays it using a json command

#include <cstdlib> 
#include <iostream> 
#include <sstream> 
#include <string> 
#include <RF24/RF24.h>
using namespace std; 


// // Hardware configuration // 
// CE Pin, CSN Pin, SPI Speed 
// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 1Mhz 
//RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS1, BCM2835_SPI_SPEED_1MHZ); 
// Setup for GPIO 15 CE and CE0 CSN with SPI Speed @ 4Mhz 
//RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_4MHZ); 
// Setup for GPIO 15 CE and CE0 CSN with SPI Speed @ 8Mhz 
RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

// The sizeof this struct should not exceed 32 bytes
struct MyData {
  float Vrms;
  float Irms;
  float Power;
  float Temp;
  float TempErr;
  unsigned long packetId; // 16 month rollover at 100 packets/sec
};

MyData data;

void storeData(void);

int main(int argc, char** argv){
  	radio.begin();
  	radio.setAutoAck(1); // Ensure autoACK is enabled
  	radio.enableAckPayload(); // Allow optional ack payloads
  	radio.setRetries(1,15); // Smallest time between retries, max no. of retries

  	radio.printDetails(); // Dump the configuration of the rf unit for debugging
  	radio.openWritingPipe(pipes[0]);
  	radio.openReadingPipe(1,pipes[1]);
	  radio.powerUp();
  	radio.startListening();
	while (1){
	   if(radio.available()){
	  	printf("Receiving..\n");
	  	radio.read(&data,sizeof(MyData));
  		radio.stopListening();
  		storeData();
  		radio.startListening();
	  }
	    delay(1500); //Delay after a response to minimize CPU usage on RPi
			//Expects a payload every second
	} //while 1
} //main

void  storeData() {
	std::ostringstream cmd;
	if(data.TempErr<1)
		cmd  << "curl \"http:/"<< "/192.168.0.8/emoncms/input/post.json?node=10&json={1:"<< data.Vrms <<",2:"<<data.Irms<<",3:"<<data.Power << ",4:" << data.Temp <<",5:"<<data.TempErr<<"}&apikey=b429eef44a575d44a37e4dd4935294e0\"";
	else
		cmd  << "curl \"http:/"<< "/192.168.0.8/emoncms/input/post.json?node=10&json={1:"<< data.Vrms <<",2:"<<data.Irms<<",3:"<<data.Power<< ",5:"<<data.TempErr <<"}&apikey=b429eef44a575d44a37e4dd4935294e0\"";
	printf("%ld \n",data.packetId);
	if(data.packetId < 1000 && data.packetId > 0)
	{
		printf("Data Stored!\n Temp: %f +- %f \n Power: %f \n packetId: %ld \n",data.Temp, data.TempErr, data.Power, data.packetId);
		system(cmd.str().c_str());
//		printf("%s", cmd.str().c_str());
	}
	else
		printf("Data corrupt. Ommiting\n");
}
