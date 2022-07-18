//// - Wifi connection
#include <PubSubClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = "FitbitLab";
const char* password = "13578077";
WiFiClient espClient;
PubSubClient client(espClient);
// Maker Webhooks IFTTT
// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://maker.ifttt.com/trigger/NFC_login/with/key/l-cpP3aqU58lMdQUHRdoKG44x3djjffjBvy2iSmZa3e";



//// - NFC
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 5
#define RST_PIN 0
#define RED 13
#define GREEN 21
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//// - Motor
#include <ESP32Servo.h>
#define Servo_PWM 17
Servo MG995_Servo;
int pos = 0; // Initial position of the servo

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

// Helper function: trigger webhook with data
void makeIFTTTRequest(bool valid_status) {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
           
    // Create HTTP request with a content type: application/json
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = "";
    if (valid_status) {
      httpRequestData = "{\"value1\" : \"valid\"}";
    } else {
      httpRequestData = "{\"value1\" : \"invalid\"}";
    }

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
      
    //Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
          
    // Free resources
    http.end();
    Serial.println("Stop connection");
  }
  else {
    Serial.println("WiFi Disconnected");
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

  // Initiate the serial communication 
  Serial.begin(115200);

  // Initiate SPI bus
  SPI.begin();

  // Initiate MFRC522 and signal LEDs
  mfrc522.PCD_Init();   
  pinMode(GREEN,OUTPUT);
  pinMode(RED, OUTPUT);   
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);  

  // Initialize the motor
  MG995_Servo.attach(Servo_PWM);

  // Initiate the Wifi connection
  setup_wifi();

  // Configure MQTT broker and listen to data
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
    // - Connect with MQTT broker
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }
    
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      return;
    }
    
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
       Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
       Serial.print(mfrc522.uid.uidByte[i], HEX);
       content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();

    // Change here the UID of the card/cards that you want to give access
    if ((content.substring(1) == "D3 B9 E4 AB") || (content.substring(1) == "D3 BE 01 AC") )
    {
      Serial.println("Authorized access");
      Serial.println();
      // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      digitalWrite(GREEN, HIGH);
      delay(3000);
      digitalWrite(GREEN, LOW);

      // Send signal to IFTTT webhook
      makeIFTTTRequest(true);

      // Send the signal to MQTT broker to turn on light
      Serial.println("Publish topic DEE_LED_control - 1");
      client.publish("DEE_LED_control", "1");

      // Open the door
      for (pos = 0; pos <= 90; pos += 1) {
        // in steps of 1 degree
        MG995_Servo.write(pos);
        delay(10);
      }

      // Wait 5s
      delay(5000);

      // Close the door
      for (pos = 90; pos >= 0; pos -= 1) {
        MG995_Servo.write(pos);
        delay(15);
      }

      // Then turn LED off
      client.publish("DEE_LED_control", "0");
   }
   else {
      Serial.println(" Access denied");
      digitalWrite(RED, HIGH);
      delay(3000);
      digitalWrite(RED, LOW);

      // Send signal to IFTTT webhook
      makeIFTTTRequest(false);
      
      // Make sure that the door is closed
      MG995_Servo.write(0);
    }
}
