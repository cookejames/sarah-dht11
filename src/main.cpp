#define VERSION "1"
#include "SarahHome.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const char* mqttTopicFormat = "sensors/%s/readings";
char mqttTopic[50];

SarahHome sarahHome("sensors");

float lastTemperature;
unsigned long lastTemperaturePublish = 0;
unsigned long timeBetweenPublish = 300000; //5 minutes

#define DHTPIN 2
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  sarahHome.setup(VERSION);
  sprintf(mqttTopic, mqttTopicFormat, sarahHome.getNodeId().c_str());

  dht.begin();
}

bool publishValue(float v, const char* topic) {
  if (isnan(v)) {
    return false;
  }
  String value;
  value += v;
  Serial.print("Sending to " + (String)mqttTopic + ": ");
  Serial.println(value);
  return sarahHome.mqttClient.publish(topic, value.c_str());
}

void publishTemperature() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;

  if (isnan(temperature)) {
    return;
  }

  if ((millis() - lastTemperaturePublish) > timeBetweenPublish ||
      (temperature > lastTemperature + 0.5) || (temperature < lastTemperature - 0.5)) {
    if (publishValue(temperature, mqttTopic)) {
        lastTemperature = temperature;
        lastTemperaturePublish = millis();
    }
  }
}

void loop() {
  sarahHome.loop();

  publishTemperature();

  delay(1000);
}
