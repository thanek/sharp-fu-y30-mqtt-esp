void initDebug(const char *hostname, void (*callback)());
void handleDebug();
String getRemoteDebugLastCommand();

void DLOG(const char *msg, ...);
void VLOG(const char *msg, ...);
