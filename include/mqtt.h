#include <WiFiClient.h>
#include <PubSubClient.h>

void mqttConnect(const char *host, int port, const char *user, const char *password, const char *clientId, MQTT_CALLBACK_SIGNATURE);

char *mqttGetStateTopic();
char *mqttGetPresetStateTopic();
char *mqttGetOscillationStateTopic();
char *mqttGetCommandTopic();

void mqttPublishState(const char* payload);
void mqttPublishPresetState(const char* payload);
void mqttPublishOscillationState(const char* payload);
void mqttPublishConfig(const char* payload);
void mqttSubscribeForCommands();
void mqttLoop();
