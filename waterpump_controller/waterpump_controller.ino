#include <esp_now.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino.h>

#define water_level_pin 34
#define water_pump_pin 33

const char* ssid = "LEO1_TEAM_02";
const char* password = "embeddedlinux";

const char* mqttServer = ""; // add rpi ip.
const int   mqttPort = 1883;
const char* mqttTopic = "feeds/moisture";
const char* waterLevelTopic = "feeds/waterlevel";

WiFiClient espClient;
PubSubClient client(espClient);

// Define variables to store sensor readings to be sent
int moisture = 0;

unsigned long millisNow = 0; // for delay purpose
unsigned int sendDelay = 2000; // delay before sending sensor info via MQTT

const int moistureThresholdHigh = 2730;
const int moistureThresholdLow = 1365;



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

void topicsSubcribe() {
  client.subscribe(mqttTopic);
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
   
    if (client.connect("WaterController")) {
      Serial.println("connected");
      topicsSubcribe();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  topicsSubcribe();
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message received from topic: ");
    Serial.println(mqttTopic);

    String byteRead = "";
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        byteRead += (char)payload[i];
    }    
    Serial.println(byteRead);

    if (String(mqttTopic) == mqttTopic) {
      Serial.print("*** (Moist Level Received)");
      moisture = byteRead.toInt();
    }
}

bool getValues() {
  if (moisture < moistureThresholdLow) {
    return 1;
  } else {
    return 0;
  }
}
void mqtt_send_water_level() {
  int moisture = analogRead(water_level_pin);
  Serial.print("Sending water level to mqtt ");
  Serial.println(moisture);
  
  char msg_out[20];
  sprintf(msg_out, "%d",moisture);
  client.publish(waterLevelTopic, msg_out, true);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  delay(1000);
  wifi_connect();
    pinMode(water_pump_pin, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void pump() {
  if (moisture <= 1000) {
    digitalWrite(water_pump_pin, HIGH);
  } else { 
    digitalWrite(water_pump_pin, LOW); 
  }
}

void loop() {
if (!client.connected()){
    reconnect();
  }

  if (millis() > millisNow + sendDelay) {
    if (getValues()) {
      mqtt_send_water_level();
      
      //digitalWrite(water_pump_pin, HIGH);
      millisNow = millis();
    }
    pump();
    millisNow = millis();
  }
  client.loop();

}
