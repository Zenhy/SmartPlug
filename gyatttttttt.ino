#include <WiFi.h>
#include <PubSubClient.h>
#include <EmonLib.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// Create instances
Preferences preferences;
EnergyMonitor emon;

// Define pins and variables
const int relayPin = 15;
const int zmptPin = 35;
const int acsPin = 32;

bool relayStatus = false;

String ssid, password, mqttServer, publishTopic, subscribeTopic;
int mqttPort;

float currentReading = 0.0;
float voltageReading = 0.0;
float realPowerReading = 0.0;

WiFiClient espClient;
PubSubClient client(espClient);

// Timing variables
unsigned long lastSensorUpdate = 0;
const unsigned long sensorInterval = 3000; // 3 seconds

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  setupPreferences();
  setupWiFi();
  setupMQTT();

  emon.current(acsPin, 5.2);  // Calibration factor for ACS712 5A
  emon.voltage(zmptPin, 57.35, 1.0);  // Updated calibration
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }

  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnectMQTT();
  }

  // Main logic (only check relay status, then act accordingly)
  if (client.connected()) {
    client.loop();  // Keep the MQTT connection alive and process incoming messages
    if (currentMillis - lastSensorUpdate >= sensorInterval) {
      lastSensorUpdate = currentMillis;
      if (relayStatus) {
        // Relay is ON, get and send sensor data
        updateSensorReadings();
        publishSensorData();
      } else {
        // Relay is OFF, send zero data
        publishZeroData();
      }
    }
  }
}

void setupPreferences() {
  preferences.begin("credStorage", false);
  if (!preferences.isKey("ssid")) {
    preferences.putString("ssid", "TELLO-98BC1C");
    preferences.putString("password", "");
    preferences.putString("mqttServer", "192.168.10.3");
    preferences.putInt("mqttPort", 1883);
    preferences.putString("publishTopic", "esp32/data");
    preferences.putString("subscribeTopic", "nodered/data");
    Serial.println("Credentials and topics saved in preferences.");
  }
  ssid = preferences.getString("ssid", "defaultSSID");
  password = preferences.getString("password", "defaultPassword");
  mqttServer = preferences.getString("mqttServer", "defaultServer");
  mqttPort = preferences.getInt("mqttPort", 1883);
  publishTopic = preferences.getString("publishTopic", "defaultPublishTopic");
  subscribeTopic = preferences.getString("subscribeTopic", "defaultSubscribeTopic");

  Serial.println("Loaded credentials and topics:");
  Serial.println("SSID: " + ssid);
}

void setupWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected.");
}

void reconnectWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setupMQTT() {
  client.setServer(mqttServer.c_str(), mqttPort);
  client.setCallback(callback);
  reconnectMQTT();
}

void reconnectMQTT() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("MQTT connected");
      client.subscribe(subscribeTopic.c_str());
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait before retrying
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String receivedPayload = "";
  for (unsigned int i = 0; i < length; i++) {
    receivedPayload += (char)payload[i];
  }

  // Toggle relay based on received message
  if (receivedPayload == "ON") {
    digitalWrite(relayPin, HIGH);
    relayStatus = true;

    client.publish(publishTopic.c_str(), "Relay Status: ON");
  } else if (receivedPayload == "OFF") {
    digitalWrite(relayPin, LOW);
    relayStatus = false;
    client.publish(publishTopic.c_str(), "Relay Status: OFF");
  }
}

void updateSensorReadings() {
  // Discard initial readings to avoid spikes
  for (int i = 0; i < 2; i++) {
    emon.calcVI(32000, 500);  // Get readings but discard them
    // delay(200);
  }

  emon.calcVI(32000, 500);  // Get readings
  voltageReading = emon.Vrms;

  currentReading = emon.Irms;
  if (voltageReading < 200) voltageReading = 0.0;
  if (currentReading < 0.1) currentReading = 0.0;

  realPowerReading = voltageReading * currentReading;

  Serial.print("Voltage (Vrms): ");
  Serial.print(voltageReading);
  Serial.print(" V, Current (Irms): ");
  Serial.print(currentReading);
  Serial.print(" A, Real Power: ");
  Serial.print(realPowerReading);
  Serial.println(" W");
}

void publishSensorData() {
  StaticJsonDocument<200> doc;
  doc["voltage"] = voltageReading;
  doc["current"] = currentReading;
  doc["realPower"] = realPowerReading;
  doc["relayStatus"] = relayStatus ? "ON" : "OFF";

  String output;
  serializeJson(doc, output);  // Serialize to string

  Serial.println("Publishing data: " + output);  // Print out the data you're publishing
  client.publish(publishTopic.c_str(), output.c_str());  // Send the data
  
  Serial.println("Data Published to MQTT");
}

void publishZeroData() {
  StaticJsonDocument<200> doc;
  doc["voltage"] = 0;
  doc["current"] = 0;
  doc["realPower"] = 0;
  doc["relayStatus"] = "OFF";

  String output;
  serializeJson(doc, output);  // Serialize to string
  
  Serial.println("Publishing zero data: " + output);  // Print out the data
  client.publish(publishTopic.c_str(), output.c_str());  // Send zero data
  
  Serial.println("Zero data Published to MQTT");
}
