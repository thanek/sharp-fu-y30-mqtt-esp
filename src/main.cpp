#include <ArduinoJson.h>
#include "config.h"
#include "mqtt.h"
#include "ota.h"

char hostname[128];

#define ON_OFF 15     // D8
#define PLASMA 13     // D7
#define MODE 12       // D6
#define MODE1 0       // D3
#define MODE3 4       // D2
#define MODE2 5       // D1
#define PLASMA_LED 16 // D0

const char *modes[] = {"off", "low", "medium", "high"};

typedef struct
{
  bool isOn;
  bool plasmaOn;
  int mode;
} SharpState;

SharpState sharpState = {};

void createEntityID()
{
  byte MAC[6];
  WiFi.macAddress(MAC);
  char MACc[30];
  sprintf(MACc, "%02X%02X%02X%02X%02X%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  sprintf(hostname, "sharp-%s", strlwr(MACc));

  Serial.print("Created hostname ");
  Serial.println(hostname);
}

void pushButton(int button)
{
  digitalWrite(button, HIGH);
  delay(100);
  digitalWrite(button, LOW);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char *cmd = (char *)malloc(length + 1);
  memcpy(cmd, payload, length);
  cmd[length] = 0;
  String command = cmd;

  if (command == "on")
  {
    if (!sharpState.isOn)
    {
      pushButton(ON_OFF);
    }
  }
  else if (command == "off")
  {
    if (sharpState.isOn)
    {
      pushButton(ON_OFF);
    }
  }
  else if (command == "plasma_on")
  {
    if (!sharpState.plasmaOn)
    {
      pushButton(PLASMA);
    }
  }
  else if (command == "plasma_off")
  {
    if (sharpState.plasmaOn)
    {
      pushButton(PLASMA);
    }
  }
  else if (command == "low")
  {
    if (sharpState.mode == 2)
    {
      pushButton(MODE);
      pushButton(MODE);
    }
    else if (sharpState.mode == 3)
    {
      pushButton(MODE);
    }
  }
  else if (command == "medium")
  {
    if (sharpState.mode == 3)
    {
      pushButton(MODE);
      pushButton(MODE);
    }
    else if (sharpState.mode == 1)
    {
      pushButton(MODE);
    }
  }
  else if (command == "high")
  {
    if (sharpState.mode == 1)
    {
      pushButton(MODE);
      pushButton(MODE);
    }
    else if (sharpState.mode == 2)
    {
      pushButton(MODE);
    }
  }
  else
  {
    char strBuf[50];
    sprintf(strBuf, "UNKNOWN COMMAND %s", cmd);
    Serial.println(strBuf);
  }

  free(cmd);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ON_OFF, OUTPUT);
  digitalWrite(ON_OFF, LOW);
  pinMode(PLASMA, OUTPUT);
  digitalWrite(PLASMA, LOW);
  pinMode(MODE, OUTPUT);
  digitalWrite(PLASMA, LOW);
  pinMode(MODE1, INPUT);
  pinMode(MODE2, INPUT);
  pinMode(MODE3, INPUT);
  pinMode(PLASMA_LED, INPUT);

  createEntityID();

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to the WiFi network");

  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  otaSetup(hostname);

  mqttConnect(MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, hostname, callback);
  mqttSubscribeForCommands();

  digitalWrite(LED_BUILTIN, HIGH);
}

long lastStatusSentTime = 0;
long lastConfigSentTime = 0;

void loop()
{
  otaHandle();
  mqttLoop();

  long now = millis();

  if (!lastConfigSentTime || now - lastConfigSentTime > 30000)
  {
    lastConfigSentTime = now;
    StaticJsonDocument<500> root;
    root["name"] = hostname;
    root["unique_id"] = hostname;
    root["icon"] = "mdi:air-filter";
    root["state_topic"] = mqttGetStateTopic();
    root["command_topic"] = mqttGetCommandTopic();
    root["oscillation_state_topic"] = mqttGetOscillationStateTopic();
    root["oscillation_command_topic"] = mqttGetCommandTopic();
    root["preset_mode_state_topic"] = mqttGetPresetStateTopic();
    root["preset_mode_command_topic"] = mqttGetCommandTopic();
    root["payload_off"] = "off";
    root["payload_on"] = "on";
    root["payload_oscillation_on"] = "plasma_on";
    root["payload_oscillation_off"] = "plasma_off";
    root["preset_modes"][0] = modes[1];
    root["preset_modes"][1] = modes[2];
    root["preset_modes"][2] = modes[3];
    String jsonStr;
    serializeJson(root, jsonStr);
    mqttPublishConfig(jsonStr.c_str());
  }

  int mode1 = digitalRead(MODE1);
  int mode2 = digitalRead(MODE2);
  int mode3 = digitalRead(MODE3);
  int plasmaState = digitalRead(PLASMA_LED);

  sharpState.isOn = (mode1 || mode2 || mode3);
  sharpState.mode = mode1 ? 1 : (mode2 ? 2 : (mode3 ? 3 : 0));
  sharpState.plasmaOn = plasmaState != 0;

  if (!lastStatusSentTime || now - lastStatusSentTime > 3000)
  {
    lastStatusSentTime = now;

    mqttPublishState(sharpState.isOn ? "on" : "off");
    mqttPublishPresetState(modes[sharpState.mode]);
    mqttPublishOscillationState(sharpState.plasmaOn ? "plasma_on" : "plasma_off");
  }
}