#include <ArduinoJson.h>
#include <WiFiManager.h>
#include "config.h"
#include "debug.h"
#include "mqtt.h"
#include "ota.h"

WiFiManager wifiManager;

char hostname[128];

#define BTN_POWER 15  // D8
#define BTN_PLASMA 13 // D7
#define BTN_MODE 12   // D6
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
}

void pushButton(int button)
{
  digitalWrite(button, HIGH);
  delay(100);
  digitalWrite(button, LOW);
  delay(100);
}

void performCommand(String command)
{
  if (command == "on")
  {
    if (!sharpState.isOn)
    {
      DLOG("Turning device ON\n");
      pushButton(BTN_POWER);
    }
  }
  else if (command == "off")
  {
    if (sharpState.isOn)
    {
      DLOG("Turning device OFF\n");
      pushButton(BTN_POWER);
    }
  }
  else if (command == "plasma_on")
  {
    if (!sharpState.plasmaOn)
    {
      DLOG("Turning Plasma Cluster ON\n");
      pushButton(BTN_PLASMA);
    }
  }
  else if (command == "plasma_off")
  {
    if (sharpState.plasmaOn)
    {
      DLOG("Turning Plasma Cluster OFF\n");
      pushButton(BTN_PLASMA);
    }
  }
  else if (command == "low")
  {
    DLOG("Switching mode to LOW\n");
    if (sharpState.mode == 2)
    {
      pushButton(BTN_MODE);
      delay(100);
      pushButton(BTN_MODE);
    }
    else if (sharpState.mode == 3)
    {
      pushButton(BTN_MODE);
    }
  }
  else if (command == "medium")
  {
    DLOG("Switching mode to MEDIUM\n");
    if (sharpState.mode == 3)
    {
      pushButton(BTN_MODE);
      delay(100);
      pushButton(BTN_MODE);
    }
    else if (sharpState.mode == 1)
    {
      pushButton(BTN_MODE);
    }
  }
  else if (command == "high")
  {
    DLOG("Switching mode to MEDIUM\n");
    if (sharpState.mode == 1)
    {
      pushButton(BTN_MODE);
      delay(100);
      pushButton(BTN_MODE);
    }
    else if (sharpState.mode == 2)
    {
      pushButton(BTN_MODE);
    }
  }
  else if (command == "version")
  {
    const char compile_date[] = __DATE__ " " __TIME__;
    DLOG("Compiled on: %s\n", compile_date);
  }
  else
  {
    DLOG("Unknown command: %s\n", command.c_str());
  }
}

void debugCallback()
{
  String cmd = getRemoteDebugLastCommand();
  DLOG("Received command via Debug interface: %s\n", cmd.c_str());
  performCommand(cmd);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char *cmd = (char *)malloc(length + 1);
  memcpy(cmd, payload, length);
  cmd[length] = 0;
  DLOG("Received command via MQTT: %s\n", cmd);
  performCommand(String(cmd));
  free(cmd);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(BTN_POWER, OUTPUT);
  digitalWrite(BTN_POWER, LOW);
  pinMode(BTN_PLASMA, OUTPUT);
  digitalWrite(BTN_PLASMA, LOW);
  pinMode(BTN_MODE, OUTPUT);
  digitalWrite(BTN_PLASMA, LOW);
  pinMode(MODE1, INPUT);
  pinMode(MODE2, INPUT);
  pinMode(MODE3, INPUT);
  pinMode(PLASMA_LED, INPUT);

  createEntityID();

  initDebug(hostname, debugCallback);

  WiFi.hostname(hostname);
  wifiManager.autoConnect();

  DLOG("Local IP: %s\n", WiFi.localIP().toString().c_str());

  otaSetup(hostname);

  WiFiManagerParameter mqttHost("server", "MQTT host", MQTT_HOST, 40);
  wifiManager.addParameter(&mqttHost);
  mqttConnect(mqttHost.getValue(), MQTT_PORT, MQTT_USER, MQTT_PASSWORD, hostname, callback);

  digitalWrite(LED_BUILTIN, HIGH);
}

bool updateSharpState()
{
  bool shouldSendState = false;
  int mode1 = digitalRead(MODE1);
  int mode2 = digitalRead(MODE2);
  int mode3 = digitalRead(MODE3);
  int plasmaState = digitalRead(PLASMA_LED);

  bool isOn = (mode1 || mode2 || mode3);
  int mode = mode1 ? 1 : (mode2 ? 2 : (mode3 ? 3 : 0));
  bool plasmaOn = plasmaState != 0;

  if (sharpState.isOn != isOn || sharpState.mode != mode || sharpState.plasmaOn != plasmaOn)
  {
    sharpState.isOn = isOn;
    sharpState.mode = mode;
    sharpState.plasmaOn = plasmaOn;
    shouldSendState = true;
  }

  return shouldSendState;
}

String generateHAConfig(const char *hostname)
{
  StaticJsonDocument<600> root;
  root["name"] = hostname;
  root["unique_id"] = hostname;
  root["icon"] = "mdi:air-filter";
  root["device"]["identifiers"][0] = hostname;
  root["device"]["manufacturer"] = "Sharp";
  root["device"]["model"] = "FU-Y30";
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
  return jsonStr;
}

long lastStatusSentTime = 0;
long lastConfigSentTime = 0;

void loop()
{
  otaHandle();
  yield();
  handleDebug();
  mqttLoop();

  bool shouldSendState = updateSharpState();

  long now = millis();

  // send config every 10 minutes so HA could have current device info
  if (!lastConfigSentTime || now - lastConfigSentTime > 600000)
  {
    lastConfigSentTime = now;
    String jsonStr = generateHAConfig(hostname);
    mqttPublishConfig(jsonStr.c_str());
  }

  // update every 10 secs or when the state actually changes
  if (shouldSendState || !lastStatusSentTime || now - lastStatusSentTime > 10000)
  {
    lastStatusSentTime = now;
    mqttPublishState(
        sharpState.isOn ? "on" : "off",
        modes[sharpState.mode],
        sharpState.plasmaOn ? "plasma_on" : "plasma_off");
  }
}