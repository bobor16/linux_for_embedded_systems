//#include <esp_now.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define pot_pin 34

const char* ssid = "LEO1_TEAM_02";
const char* password = "embeddedlinux";


const char* mqttServer = ""; // Add rpi ip.
const int mqttPort = 1883;
const char* mqttTopic = "feeds/moisture";

WiFiClient espClient;
PubSubClient client(espClient);

float sensorValue = 0; 

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

void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!client.connected()) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
   
    if (client.connect("GardenController")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

void mqtt_send_moisture() {
  int moisture = analogRead(pot_pin);
  Serial.print("Sending moisture level to mqtt");
  Serial.println(moisture);

  char msg_out[20];
  sprintf(msg_out, "%d",moisture);
  client.publish(mqttTopic, msg_out, true);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  wifi_connect();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  client.setServer(mqttServer, mqttPort);
}
 
void loop() {
 if (!client.connected()){
    reconnect();
  }
  mqtt_send_moisture();
  delay(5000);
}
