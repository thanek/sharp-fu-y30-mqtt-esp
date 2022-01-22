#include <WiFiClient.h>
#include <PubSubClient.h>

void mqttConnect(const char *host, int port, const char *user, const char *password, const char *clientId, MQTT_CALLBACK_SIGNATURE);

void mqttPublish(const char* payload);
void mqttSubscribeForCommands();
void mqttLoop();
