#include <stdio.h>
#include "mqtt.h"

char commandTopic[256];
char statusTopic[256];

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void createStatusTopic(const char *hostname)
{
  sprintf(statusTopic, "xis/%s/state", hostname);
}

void createCommandTopic(const char *hostname)
{
  sprintf(commandTopic, "xis/%s/command", hostname);
}

void mqttConnect(const char *host, int port, const char *user, const char *password, const char *clientId, MQTT_CALLBACK_SIGNATURE)
{
  createStatusTopic(clientId);
  createCommandTopic(clientId);
  
  mqttClient.setServer(host, port);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected())
  {
    Serial.printf("The client %s connects to the MQTT broker\n", clientId);
    if (mqttClient.connect(clientId, user, password))
    {
      Serial.println("MQTT broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void mqttPublish(const char *payload)
{
  mqttClient.publish(statusTopic, payload);
}

void mqttSubscribeForCommands()
{
  mqttClient.subscribe(commandTopic);
}

void mqttLoop()
{
  mqttClient.loop();
}
