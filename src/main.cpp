#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "config.h"

const char *hostname = "sharp-fu-y30eu-w";
const char *commandTopic = "xis/esp8266/cmd";
const char *statusTopic = "xis/esp8266/status";

#define ON_OFF 15 // D8
#define PLASMA 13 // D7
#define MODE 12   // D6
#define MODE1 0   // D3
#define MODE3 4   // D2
#define MODE2 5   // D1

WiFiClient espClient;
PubSubClient client(espClient);

void pushButton(int button)
{
  digitalWrite(button, HIGH);
  delay(100);
  digitalWrite(button, LOW);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

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
    client.publish(statusTopic, "Power button pressed");
  }
  else if (command == "plasma")
  {
    pushButton(PLASMA);
    client.publish(statusTopic, "Plasma button pressed");
  }
  else if (command == "mode")
  {
    pushButton(MODE);
    client.publish(statusTopic, "Mode button pressed");
  }
  else
  {
    char strBuf[50];
    sprintf(strBuf, "UNKNOWN COMMAND %s", cmd);
    Serial.println(strBuf);
    client.publish(statusTopic, strBuf);
  }

  free(cmd);
}

void setup()
{
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

  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  // Set Hostname.
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

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]()
                     { Serial.println("OTA Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nOTA End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                         Serial.println("Auth Failed");
                       else if (error == OTA_BEGIN_ERROR)
                         Serial.println("Begin Failed");
                       else if (error == OTA_CONNECT_ERROR)
                         Serial.println("Connect Failed");
                       else if (error == OTA_RECEIVE_ERROR)
                         Serial.println("Receive Failed");
                       else if (error == OTA_END_ERROR)
                         Serial.println("End Failed");
                     });
  ArduinoOTA.begin();

  //connecting to a mqtt broker
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
  while (!client.connected())
  {
    String client_id = "sharp-fu-y30-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // publish and subscribe
  client.publish(statusTopic, "hello from sharp-fu-y30");
  String msg = "My mac address: ";
  msg += String(WiFi.macAddress());
  client.publish(statusTopic, msg.c_str());

  client.subscribe(commandTopic);
}

long lastStatusSentTime = 0;

void loop()
{
  ArduinoOTA.handle();
  client.loop();
  long now = millis();
  if (now - lastStatusSentTime > 3000)
  {
    lastStatusSentTime = now;
    int mode1 = digitalRead(MODE1);
    int mode2 = digitalRead(MODE2);
    int mode3 = digitalRead(MODE3);

    String mode = mode1 ? "low" : (mode2 ? "medium" : (mode3 ? "high" : "OFF"));

    char strBuf[50];
    sprintf(strBuf, "Current mode: %s", mode.c_str());
    client.publish(statusTopic, strBuf);
  }
}