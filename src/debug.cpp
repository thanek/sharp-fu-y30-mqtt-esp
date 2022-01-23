#include <RemoteDebug.h>
#include <stdio.h>

RemoteDebug Debug;

void initDebug(const char *hostname, void (*callback)())
{
    Debug.begin(hostname);
    Debug.showTime(true);
    Debug.setResetCmdEnabled(true);
    Debug.setCallBackProjectCmds(callback);
    Debug.setSerialEnabled(false);
}

void handleDebug()
{
    Debug.handle();
}

void debugPrintf(const char *fmt, va_list argp)
{
    char buf[512];
    vsprintf(buf, fmt, argp);
    Debug.print(buf);
}

void DLOG(const char *msg, ...)
{
    if (Debug.isActive(Debug.DEBUG))
    {
        va_list argp;
        va_start(argp, msg);
        debugPrintf(msg, argp);
        va_end(argp);
    }
}

void VLOG(const char *msg, ...)
{
    if (Debug.isActive(Debug.VERBOSE))
    {
        va_list argp;
        va_start(argp, msg);
        debugPrintf(msg, argp);
        va_end(argp);
    }
}

String getRemoteDebugLastCommand()
{
    return Debug.getLastCommand();
}