#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp> //Needed to send IR information
#include <WebSocketsClient.h> //Needed in order to send any received information
#include <WiFi.h> //Needed in order to send any received information

#include "esp_system.h"//Needed to get the mac address


#define USE_SERIAL Serial1
#define DISABLE_CODE_FOR_RECEIVER
#define CONNECTION_TIMEOUT 10

Adafruit_MPU6050 mpu;

const char *ssid = "scrime-local";
const char *password = "" ;
const char *websocket_server_host = "192.168.1.2";
const uint16_t websocket_server_port = 8080;

const int volumeButtonPin = 25;
const int selectPin = 27;
const int unSelectPin = 13;



WebSocketsClient webSocket; 

String getMacAddress() {
  uint8_t baseMac[6];  
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}


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
    digitalWrite(2,HIGH);
    delay(200);
    digitalWrite(2,LOW);
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

String macAddress = getMacAddress();

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

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}
}

//there is a pull up resistance for the buttons, that is why we check if the received signal is LOW and not HIGH
bool isButtonPushed(int pin) {
  return (digitalRead(pin) == LOW); 
}

void printIrInfo(uint8_t sCommand, uint8_t sRepeats) {
  Serial.println();
  Serial.print(F("Send now: address=0x00, command=0x"));
  Serial.print(sCommand, HEX);
  Serial.print(F(", repeats="));
  Serial.print(sRepeats);
  Serial.println();
  Serial.println(F("Send standard NEC with 8 bit address"));
  Serial.flush();   
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(volumeButtonPin, INPUT_PULLUP);
  pinMode(selectPin, INPUT_PULLUP);
  pinMode(unSelectPin, INPUT_PULLUP);
  Serial.begin(115200);

  IrSender.begin(DISABLE_LED_FEEDBACK); // Start with IR_SEND_PIN as send pin and disable feedback LED at default feedback LED pin

  while (!Serial) {}
    delay(10); // will pause Zero, Leonardo, etc until serial console opens


  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

//Initialize Wifi and websocket connexion
  WiFi.mode(WIFI_STA);
  
  webSocket.begin(websocket_server_host,websocket_server_port,"/");

  webSocket.onEvent(webSocketEvent);

  webSocket.setReconnectInterval(5000);
}


uint8_t sCommand = 0x32;//arbitrary value 
uint8_t sRepeats = 4;


void loop() {
  webSocket.loop();
  if (isConnectedToWifi()) {
    if (webSocket.isConnected()) {
      digitalWrite(2,LOW);
      while (isButtonPushed(selectPin)) {
        sCommand = 0x10; //the value is abritray but needs to be coherent with what's noted on highspeaker related esp script 
        printIrInfo(sCommand, sRepeats);
        IrSender.sendNEC(0x00, sCommand, sRepeats);
        delay(1000); //mandatory between two sent messages
      }
      while (isButtonPushed(unSelectPin)) {
        sCommand = 0x11;//the value is abritray but needs to be coherent with what's noted on highspeaker related esp script 
        printIrInfo(sCommand, sRepeats);
        IrSender.sendNEC(0x00, sCommand, sRepeats);
        delay(1000); //mandatory between two sent messages
      }
      while (isButtonPushed(volumeButtonPin)) {
        unsigned long now = millis();
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        unsigned long afterNow = millis();
        if (afterNow > now) {
          /*
          Serial.print("Gyrox:");
          Serial.print(g.gyro.x);Serial.print(",");
        // Serial.print(", Y: ");  Serial.print(",");
          Serial.print("Gyroy:");
          Serial.print(g.gyro.y);Serial.print(",");
        // Serial.print(", Z: ");  Serial.print(",");
          Serial.print("Gyroz:");
          Serial.print(g.gyro.z);
          */
          unsigned long dt = afterNow - now;
          webSocket.sendTXT(macAddress + "\n" + "Volume" + "\n" + -g.gyro.x + "\n" + "dt" + "\n" + dt); //gyroscope was built the wrong way, the minus g.gyrox is there to fix that
        }
      }
    delay(50);
    }
    else {
      //if not connected yet to the webscoket server but to the wifi, the esp led lights up 
      digitalWrite(2,HIGH);
    }
  }
  else {
    //if not connected yet to the wifi, the esp led blinks 
    connectionToWifi(ssid, password);
  }
}
