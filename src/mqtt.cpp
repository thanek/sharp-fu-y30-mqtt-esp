#include <stdio.h>
#include "mqtt.h"

char commandTopic[256];
char stateTopic[256];
char presetStateTopic[256];
char oscillationStateTopic[256];
char configTopic[256];

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void generateTopicNames(const char *clientId)
{
  const char *topicPrefix = "homeassistant/fan/";
  sprintf(stateTopic, "%s%s/state", topicPrefix, clientId);
  sprintf(presetStateTopic, "%s%s/preset", topicPrefix, clientId);
  sprintf(oscillationStateTopic, "%s%s/oscillation", topicPrefix, clientId);
  sprintf(commandTopic, "%s%s/command", topicPrefix, clientId);
  sprintf(configTopic, "%s%s/config", topicPrefix, clientId);
}

void mqttConnect(const char *host, int port, const char *user, const char *password, const char *clientId, MQTT_CALLBACK_SIGNATURE)
{
  generateTopicNames(clientId);

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

void mqttPublishState(const char *payload)
{
  mqttClient.publish(stateTopic, payload);
}

void mqttPublishPresetState(const char *payload)
{
  mqttClient.publish(presetStateTopic, payload);
}

void mqttPublishOscillationState(const char *payload)
{
  mqttClient.publish(oscillationStateTopic, payload);
}

void mqttPublishConfig(const char *payload)
{
  mqttClient.publish(configTopic, payload);
}

void mqttSubscribeForCommands()
{
  mqttClient.subscribe(commandTopic);
}

void mqttLoop()
{
  mqttClient.loop();
}

char *mqttGetStateTopic()
{
  return stateTopic;
}

char *mqttGetPresetStateTopic()
{
  return presetStateTopic;
}

char *mqttGetOscillationStateTopic()
{
  return oscillationStateTopic;
}

char *mqttGetCommandTopic()
{
  return commandTopic;
}