#include <esp_now.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino.h>

#define RELAY_PIN 0 // ESP32 pin 0, which connects to the IN pin of relay
#define echoPin 5 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 18 //attach pin D3 Arduino to pin Trig of HC-SR04

const char* ssid = "LEO1_TEAM_02";
const char* password = "embeddedlinux";
const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttUser = "bobor16";
const char* mqttPassword = "aio_MscP52tsMjUOfPf2tQqk4W9aflSS";
const char* mqttTopic = "bobor16/feeds/moisture";

WiFiClient espClient;
PubSubClient client(espClient);

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x84, 0x0D, 0x8E, 0xE4, 0xAB, 0x00};

// Define variables to store sensor readings to be sent
float moisture;

// Define variables to store incoming readings
float incomingMoist;

// Variable to store if sending data was successful
String success;

//--------- WIFI -------------------------------------------

void wifi_connect() {
  Serial.print("Starting connecting WiFi.");
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//------------------ MQTT ----------------------------------
void mqtt_setup() {
  client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    Serial.println("Connecting to MQTT…");
    while (!client.connected()) {        
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("connected");
        } else {
            Serial.print("failed with state  ");
            Serial.println(client.state());
            delay(2000);
        }
    }
//    mqtt_send_moisture();
    client.subscribe(mqttTopic);
}

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived in topic: ");
    Serial.println(topic);

    String byteRead = "";
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        byteRead += (char)payload[i];
    }    
    Serial.println(byteRead);
}
/*
void mqtt_send_moisture() {
  int moisture = analogRead(pot_pin);
  Serial.print("Sending moisture level to mqtt");
  Serial.println(moisture);

  char msg_out[20];
  sprintf(msg_out, "%d",moisture);
  client.publish(mqttTopic, msg_out);
}
*/
//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float moist;
} struct_message;

// Create a struct_message called sensorReadings to hold sensor readings
struct_message sensorReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
 // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  incomingMoist = incomingReadings.moist;
}
 
void setup() {

  // Init Serial Monitor
  Serial.begin(115200);

  wifi_connect();
  mqtt_setup();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
    client.loop();

  
  //Serial.println("recieved with esp_now");
  //Serial.println(incomingMoist);
  // Set values to send
  // sensorReadings.moist = 11;

/*

  // Send message via ESP-NOW
   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sensorReadings, sizeof(sensorReadings));
   
  if (result == ESP_OK) {
    //Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  */
  //delay(10000);
}
