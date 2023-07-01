#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define WIFISSID "homeiot"
#define PASSWORD "homeiot123"
#define TOKEN "BBFF-f3KGhv8nAjMcaDiCYLAfuWEZSq8C10"
#define MQTT_CLIENT_NAME "myecgsensor2"

#define ECG_VARIABLE_LABEL "myecg"
#define DHT_TEMPERATURE_VARIABLE_LABEL "mytemp"
#define PULSE_SENSOR_VARIABLE_LABEL "mypulse"
#define DEVICE_LABEL "esp32"

#define ECG_SENSOR 34 // Connect ECG sensor to GPIO 34 (Analog pin)
#define DHT_PIN 4 // Connect DHT11 data pin to GPIO 4
#define PULSE_SENSOR_PIN 36 // Connect Pulse sensor to GPIO 36 (Analog pin)

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[300];
char topic[150];
char str_sensor[10];

WiFiClient ubidots;
PubSubClient client(ubidots);

DHT dht(DHT_PIN, DHT11);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  pinMode(ECG_SENSOR, INPUT);
  pinMode(PULSE_SENSOR_PIN, INPUT);
  dht.begin();
  
  Serial.println();
  Serial.print("Waiting for WiFi...");
   
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
   
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  // Publish ECG value
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", "");
  sprintf(payload, "{\"%s\":", ECG_VARIABLE_LABEL);
  float ecg_value = analogRead(ECG_SENSOR);
  dtostrf(ecg_value, 4, 2, str_sensor);
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor);
  Serial.println("Publishing ECG data to Ubidots Cloud");
  client.publish(topic, payload);
  
  // Publish temperature value
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", "");
  sprintf(payload, "{\"%s\":", DHT_TEMPERATURE_VARIABLE_LABEL);
  float temperature = dht.readTemperature();
  dtostrf(temperature, 4, 2, str_sensor);
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor);
  Serial.println("Publishing temperature data to Ubidots Cloud");
  client.publish(topic, payload);

    // Publish Pulse value
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); 
  sprintf(payload, "{\"%s\":", PULSE_SENSOR_VARIABLE_LABEL);
  float pulse_value = (analogRead(PULSE_SENSOR_PIN)/27);
  dtostrf(pulse_value, 4, 2, str_sensor);
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor);
  Serial.println("Publishing Pulse data to Ubidots Cloud");
  client.publish(topic, payload);
  
  delay(1000); // Publish data every 1 second
}
