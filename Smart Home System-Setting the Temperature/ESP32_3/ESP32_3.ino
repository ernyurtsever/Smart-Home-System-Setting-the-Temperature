// Expect to work on ESP32_3

//// - Motor
#include <ESP32Servo.h>
#define Servo_PWM 17
Servo MG995_Servo;
int pos = 0; // Initial position of the servo

//// - LED
#define LED_pin 19

//// - Wifi connection
#include <PubSubClient.h>
#include <WiFi.h>
const char* ssid = "FitbitLab";
const char* password = "13578077";
WiFiClient espClient;
PubSubClient client(espClient);

//// - MQTT
const char* mqtt_server = "test.mosquitto.org";

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void motor_control(int delay_time) {
  // Attach the motor
  MG995_Servo.attach(Servo_PWM);

  // Simulate the opearting of fan
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    MG995_Servo.write(pos);
    delay(delay_time);
  }
  
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    MG995_Servo.write(pos);
    delay(delay_time);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, "DEE_motor_control") == 0) {
      if ((char)payload[0] == '1') {
        motor_control(2);
      } else {
        MG995_Servo.detach();
      }
  } else if (strcmp(topic, "DEE_LED_control") == 0){
      if ((char)payload[0] == '1') {
        digitalWrite(LED_pin, HIGH);
      } else {
        digitalWrite(LED_pin, LOW);
      }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, subscribe to "DEE_motor_control"
      client.subscribe("DEE_motor_control");
      client.subscribe("DEE_LED_control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_pin, OUTPUT);   
  digitalWrite(LED_pin, LOW);
  
  // Configure MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
