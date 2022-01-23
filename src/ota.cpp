#include <stdio.h>
#include "ota.h"

void otaSetup(const char *hostname)
{
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.onStart([]()
                       { DLOG("OTA Start\n"); });
    ArduinoOTA.onEnd([]()
                     { DLOG("\nOTA End\n"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { DLOG("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           DLOG("Error[%u]: \n", error);
                           if (error == OTA_AUTH_ERROR)
                               DLOG("Auth Failed\n");
                           else if (error == OTA_BEGIN_ERROR)
                               DLOG("Begin Failed\n");
                           else if (error == OTA_CONNECT_ERROR)
                               DLOG("Connect Failed\n");
                           else if (error == OTA_RECEIVE_ERROR)
                               DLOG("Receive Failed\n");
                           else if (error == OTA_END_ERROR)
                               DLOG("End Failed\n");
                       });
    ArduinoOTA.begin();
}

void otaHandle()
{
    ArduinoOTA.handle();
}