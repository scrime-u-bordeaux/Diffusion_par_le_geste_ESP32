#include <IRremote.hpp> //Needed in order to receive IR information
#include "PinDefinitionsAndMore.h"
#include <Arduino.h>

//Needed in order to send any received information
#include <WebSocketsClient.h>
#include <WiFi.h>
//Needed to get the mac address
#include "esp_system.h"

#define USE_SERIAL Serial1
#define CONNECTION_TIMEOUT 10
#define DECODE_NEC

//Properties of the Wifi and the Websocket server
const char *ssid = "scrime-local";
const char *password = "" ;
const char *websocket_server_host = "192.168.1.2";
const uint16_t websocket_server_port = 8080;

const int greenPin = 25;
const int redPin = 27;

WebSocketsClient webSocket; 

String getMacAddress() {
  uint8_t baseMac[6];  
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}

String macAddress = getMacAddress();

//functions related to the Wifi
bool isConnectedToWifi() {
  return (WiFi.status() == WL_CONNECTED);
}

void printIp() {
  Serial.print("IP :");
  Serial.println(WiFi.localIP());
}

void connectionToWifi(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
  int timeout_counter = 0;
  while(WiFi.status() != WL_CONNECTED){
	  Serial.print(".");
	  delay(200);
	  timeout_counter++;
	  if(timeout_counter >= CONNECTION_TIMEOUT*5){
	    ESP.restart();
	  }
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
      webSocket.begin(websocket_server_host,websocket_server_port,"/");
			break;
		case WStype_CONNECTED:
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
			// send message to server when Connected
			webSocket.sendTXT("ESP connectée : " + macAddress);
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      /*
			// send message to server
			// webSocket.sendTXT("message here");
			break;*/
		case WStype_BIN:
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}
}


void setup() {
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.begin(115200);

//initialize IR reception
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

//initialize the connection with the websocket
  WiFi.mode(WIFI_STA);
  webSocket.begin(websocket_server_host,websocket_server_port,"/");

  webSocket.onEvent(webSocketEvent);

  webSocket.setReconnectInterval(5000);
}  

void loop() {
  webSocket.loop();
  if (isConnectedToWifi()) {
    if (webSocket.isConnected()) {
      digitalWrite(2,LOW);
      if (IrReceiver.decode()) {
        uint16_t message = IrReceiver.decodedIRData.command;
        if ((IrReceiver.decodedIRData.protocol != UNKNOWN) && ((message == 0x10) || (message == 0x11))) {
          int boolean = (message == 0x10) ? 1 : 0;
          //Serial.println("Received something: " +String(message));
          webSocket.sendTXT(macAddress + "\n" + "receiving" + "\n" + String(boolean));
          if (boolean) {
            digitalWrite(greenPin, HIGH);
            digitalWrite(redPin, LOW);
          }
          else {
            digitalWrite(greenPin, LOW);
            digitalWrite(redPin, HIGH);
          }
        }        
        IrReceiver.resume();
      } 
    }
    else {
      //if not connected to the websocket yet but to the wifi, the esp led lights up 
      digitalWrite(2,HIGH);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW); 
    }
  }
  else {
    //if not connected to the wifi, both greend and red leds light up 
    digitalWrite(2,LOW);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    connectionToWifi(ssid,password);
  }
}