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
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if (command == "off")
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (command == "power")
  {
    pushButton(ON_OFF);
    mqttPublish("Power button pressed");
  }
  else if (command == "plasma")
  {
    pushButton(PLASMA);
    mqttPublish("Plasma button pressed");
  }
  else if (command == "mode")
  {
    pushButton(MODE);
    mqttPublish("Mode button pressed");
  }
  else
  {
    char strBuf[50];
    sprintf(strBuf, "UNKNOWN COMMAND %s", cmd);
    Serial.println(strBuf);
    mqttPublish(strBuf);
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

void loop()
{
  otaHandle();
  mqttLoop();

  long now = millis();
  if (now - lastStatusSentTime > 3000)
  {
    lastStatusSentTime = now;
    int mode1 = digitalRead(MODE1);
    int mode2 = digitalRead(MODE2);
    int mode3 = digitalRead(MODE3);
    int plasmaState = digitalRead(PLASMA_LED);

    String mode = mode1 ? "low" : (mode2 ? "medium" : (mode3 ? "high" : "OFF"));
    String plasma = plasmaState ? "ON" : "OFF";

    char strBuf[50];
    sprintf(strBuf, "Current mode: %s, Plasma: %s", mode.c_str(), plasma.c_str());
    mqttPublish(strBuf);
  }
}