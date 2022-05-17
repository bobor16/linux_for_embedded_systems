#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#define SensorPin 34
float sensorValue = 0; 

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x10, 0x97, 0xBD, 0xD5, 0xB2, 0x70};

// Define variables to store sensor readings to be sent
float moisture;
// Define variables to store incoming readings
float incomingmoist;

// Variable to store if sending data was successful
String success;

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
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    //success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingmoist = incomingReadings.moist;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  
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

}
 
void loop() {
  Serial.println("Sensor readings:");
  float moisture = analogRead(SensorPin);
  //moisture = moisture/2500*100;
 // Serial.println(moisture/2300*100);
   //Serial.println(String(moisture, 2) + String("% Moist"));
  Serial.println(moisture);
  
  getReadings();

  // Set values to send
  sensorReadings.moist = moisture;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sensorReadings, sizeof(sensorReadings));
   
  if (result == ESP_OK) {
   // Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  //delay(10000);
}
void getReadings(){
  moisture = digitalRead(SensorPin);
  delay(100);
}

void updateDisplay(){
  
  // Display Readings in Serial Monitor
  Serial.println("INCOMING READINGS");
  Serial.print("moisture: ");
  Serial.print(incomingReadings.moist);
  Serial.println("%");
}
