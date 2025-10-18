#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <ESP_Mail_Client.h>

// -------------------- Wi-Fi credentials --------------------
const char *WIFI_SSID     = "cabdullecismaancadde";   // CHANGE TO YOUR WIFI SSID
const char *WIFI_PASSWORD = "Mohamed7.";   // CHANGE TO YOUR WIFI PASSWORD

// -------------------- MQTT broker settings --------------------
const char MQTT_BROKER_ADRRESS[] = "192.168.43.50";
const int MQTT_PORT              = 1883;
const char MQTT_CLIENT_ID[]      = "Sonwabise-esp32-001";
const char MQTT_USERNAME[]       = "lab";
const char MQTT_PASSWORD[]       = "t4b_grp02";

// -------------------- MQTT topics --------------------
const char PUBLISH_TOPIC[]   = "motion/sensor6";
const char SUBSCRIBE_TOPIC[] = "motion/sensor6";

// -------------------- Sensor and buzzer pins --------------------
const int PIR_SENSOR_PIN = 13;   // GPIO13 connected to PIR motion sensor
const int BUZZER_PIN     = 23;   // GPIO23 connected to buzzer

// -------------------- Publish timing --------------------
const int PUBLISH_INTERVAL = 5000; // Publish every 5s

// -------------------- Email configuration --------------------
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL    "4340789@myuwc.ac.za"      // Replace with your Gmail
#define AUTHOR_PASSWORD "cqtb jzuo tolw rvbd"         // Replace with Gmail App Password
#define RECIPIENT_EMAIL "oluhlembalo16@gmail.com" // Who should get alerts

SMTPSession smtp;
unsigned long lastEmailSent = 0;
const unsigned long EMAIL_INTERVAL = 10UL * 60UL * 1000UL; // 10 minutes

// -------------------- Global state --------------------
WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

unsigned long lastPublishTime = 0;
int motionStateCurrent  = LOW;
int motionStatePrevious = LOW;

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("ESP32 - Connecting to Wi-Fi");

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nESP32 - Wi-Fi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    connectToMQTT();
  } else {
    Serial.println("\nESP32 - Failed to connect to Wi-Fi");
    while (true) {
      delay(1000); 
      Serial.println("Couldn't connect to Wifi");  
    }
  }
} 

// -------------------- Main loop --------------------
void loop() {
  mqtt.loop();

  motionStatePrevious = motionStateCurrent;
  motionStateCurrent = digitalRead(PIR_SENSOR_PIN);

  // Detect state changes
  if (motionStatePrevious != motionStateCurrent) {
    if (motionStateCurrent == HIGH) {
      Serial.println("Motion detected!, making sound");
      digitalWrite(BUZZER_PIN, HIGH);
      sendEmailWithRateLimit();  // Try to send email (if enough time has passed)
    } else {
      Serial.println("Motion stopped!, stops making sound");
      digitalWrite(BUZZER_PIN, LOW);
    }
    sendToMQTT(motionStateCurrent);
  }

  // Optional periodic keep-alive publish
  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sendToMQTT(motionStateCurrent);
    lastPublishTime = millis();
  }
}

// -------------------- Connect to MQTT --------------------
void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageHandler);

  Serial.print("ESP32 - Connecting to MQTT broker");

  while (!mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  if (!mqtt.connected()) {
    Serial.println("ESP32 - MQTT broker Timeout!");
    return;
  }

  if (mqtt.subscribe(SUBSCRIBE_TOPIC))
    Serial.print("ESP32 - Subscribed to the topic: ");
  else
    Serial.print("ESP32 - Failed to subscribe to the topic: ");

  Serial.println(SUBSCRIBE_TOPIC);
  Serial.println("ESP32 - MQTT broker Connected!");
}

// -------------------- Publish to MQTT --------------------
void sendToMQTT(int motionState) {
  StaticJsonDocument<200> message;
  message["sensor_id"] = MQTT_CLIENT_ID;
  message["timestamp"] = millis();
  message["motion"]    = motionState == HIGH ? "detected" : "stopped";

  char messageBuffer[256];
  serializeJson(message, messageBuffer);

  mqtt.publish(PUBLISH_TOPIC, messageBuffer);

  Serial.println("ESP32 - sent to MQTT:");
  Serial.print("- topic: ");
  Serial.println(PUBLISH_TOPIC);
  Serial.print("- payload: ");
  Serial.println(messageBuffer);
}

// -------------------- Send Email with Rate Limit --------------------
void sendEmailWithRateLimit() {
  unsigned long now = millis();
  if (now - lastEmailSent < EMAIL_INTERVAL) {
    Serial.println("Skipping email (rate limit active)");
    return;
  }

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "ESP32 Motion Sensor";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Motion Alert!";
  message.addRecipient("User", RECIPIENT_EMAIL);

  String htmlMsg = "Motion detected by ESP32 sensor at " + String(millis() / 1000) + " seconds uptime.";
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session)) {
    Serial.println("Failed to connect to mail server");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.print("Error sending Email, ");
    Serial.println(smtp.errorReason());
  } else {
    Serial.println("Email sent successfully!");
    lastEmailSent = now;
  }
}

// -------------------- Handle MQTT messages --------------------
void messageHandler(String &topic, String &payload) {
  Serial.println("ESP32 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
  Serial.println(payload);
}
