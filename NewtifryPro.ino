#if ARDUINO >= 100
#include "Arduino.h"
#else
extern "C" {
    #include "WConstants.h"
}
#endif


#include <SPI.h>
#include <Ethernet.h>
#include "JsonGenerator.h"
using namespace ArduinoJson::Generator;
#include "Base64.h"
void buildMessage(		char *deviceId,
						char *title, 
						char *source, 
						char  *message);
void sendToNewtifryPro(char *apikey);

char *encode(char *input) {
	int len = strlen(input);
	char *encodeBuffer = new char[base64_enc_len(len)];
	base64_encode(encodeBuffer, input, len);
	return encodeBuffer;
}

/*
char *encodeBuffer = NULL;
char *encode(char *input) {
	if (encodeBuffer != NULL) {
		delete encodeBuffer;
	}
	int len = strlen(input);
	encodeBuffer = new char[base64_enc_len(len)];
	base64_encode(encodeBuffer, input, len);
	return encodeBuffer;
}
*/
  /////////////////
 // MODIFY HERE //
/////////////////
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x19 };   // Be sure this address is unique in your network

//Enter your secret API_key of your project, the RegID of the respective Android app on your specific device
//and your data
char APIKey[] = "YOUR_API_KEY"; 
char deviceID[] = "YOUR_DEVICE_ID";

// Debug mode
boolean DEBUG = true;
  //////////////
 //   End    //
//////////////


char serverName[] = "android.googleapis.com"; //GCM server address

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
#include <utility/w5100.h>

void setup() {
	Serial.begin(115200);
	// start the Ethernet connection:
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while(true);
	} else {
		W5100.setRetransmissionTime(0x07D0);
		W5100.setRetransmissionCount(10);
		Serial.println("Ethernet ready");
		
		// print the Ethernet board/shield's IP address:
		Serial.print("My IP address: ");
		Serial.println(Ethernet.localIP());
	}
	// give the Ethernet shield a second to initialize:
	delay(3000);
	buildMessage(deviceID, "test", "arduino", "message de test");
	sendToNewtifryPro(APIKey); 
}

void loop() {
}
JsonObject<4> data; 
JsonArray<1> devId; ; 
JsonObject<2> msg; 
char buffer[350];  
int inChar;

void buildMessage(	char *deviceId,
					char *title, 
					char *source, 
					char  *message) {
	if (title == NULL) {
		return;
	}
    data["type"] = "ntp_message";
	if (source != NULL) {
	    data["source"] = encode(source);							
	} else {
	    data["source"] = "";							
	}
    data["title"] = encode(title);
	if (message != NULL) {
		data["message"] = encode(message);
	} else {
		data["message"] = "";
	}
	devId.add(deviceId);
	msg["registration_ids"] = devId;
	msg["data"] = data;
	msg.printTo(buffer, sizeof(buffer));
	Serial.println(strlen(buffer));
	Serial.println(msg);
}

//Function for sending the request to GCM
void sendToNewtifryPro(char *apiKey) {
///////////////////////////////////////////////////////	
	if(DEBUG) {
		Serial.println("connecting...");
	}
	while(client.connect(serverName, 80) == false) {
		if(DEBUG){
			Serial.println("connection failed");
		}
	}
		if(DEBUG) {
			Serial.println("connected");
			Serial.println("sending request");
		}

		client.println("POST /gcm/send HTTP/1.1"); // http POST request
		client.print("Host: ");
		client.println(serverName);
		client.println("User-Agent: Arduino");
		client.println("Connection: close");
		client.println("Content-Type: application/json");
		client.print("Authorization:key=");
		client.println(apiKey);
		client.print("Content-Length: "); // has to be exactly the number of characters (bytes) in the POST body
		client.println(strlen(buffer)); // calculate content-length    
		client.println();
		client.println(buffer);
		Serial.println("message sent");
		int connectLoop = 0;

		while(client.connected()){
    		while(client.available()) {
      			inChar = client.read();
      			Serial.write(inChar);
      			// set connectLoop to zero if a packet arrives
      			connectLoop = 0;
    		}

    		connectLoop++;

    		// if more than 10000 milliseconds since the last packet
    		if(connectLoop > 20000) {
      			// then close the connection from this end.
      			Serial.println();
      			Serial.println(F("Timeout"));
      			client.stop();
    		}
    		// this is a delay for the connectLoop timing
    		delay(1);
  		}

  		Serial.println();

  		Serial.println(F("disconnecting."));
  		// close client end
  		client.stop();
}

