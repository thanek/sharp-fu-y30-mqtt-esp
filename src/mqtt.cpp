#include <stdio.h>
#include "mqtt.h"
#include "debug.h"

#define TOPIC_PREFIX "homeassistant/fan/"

char commandTopic[256];
char stateTopic[256];
char presetStateTopic[256];
char oscillationStateTopic[256];
char configTopic[256];

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void generateTopicNames(const char *clientId)
{
  sprintf(stateTopic, "%s%s/state", TOPIC_PREFIX, clientId);
  sprintf(presetStateTopic, "%s%s/preset", TOPIC_PREFIX, clientId);
  sprintf(oscillationStateTopic, "%s%s/oscillation", TOPIC_PREFIX, clientId);
  sprintf(commandTopic, "%s%s/command", TOPIC_PREFIX, clientId);
  sprintf(configTopic, "%s%s/config", TOPIC_PREFIX, clientId);
}

void mqttConnect(const char *host, int port, const char *user, const char *password, const char *clientId, MQTT_CALLBACK_SIGNATURE)
{
  generateTopicNames(clientId);

  mqttClient.setServer(host, port);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected())
  {
    DLOG("The client %s connects to the MQTT broker\n", clientId);
    if (mqttClient.connect(clientId, user, password))
    {
      DLOG("MQTT broker connected\n");
    }
    else
    {
      DLOG("failed with state %s\n", mqttClient.state());
      delay(2000);
    }
  }
  mqttClient.subscribe(commandTopic);
}

void mqttPublishState(const char *state, const char *modeName, const char *plasmaState)
{
  DLOG("Publishing state: %s, mode: %s, plasmaState: %s\n", state, modeName, plasmaState);

  mqttClient.publish(stateTopic, state);
  mqttClient.publish(presetStateTopic, modeName);
  mqttClient.publish(oscillationStateTopic, plasmaState);
}

void mqttPublishConfig(const char *payload)
{
  mqttClient.publish(configTopic, payload);
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