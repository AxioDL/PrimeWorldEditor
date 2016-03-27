#include "CTimer.h"
#include "Log.h"
#include "TString.h"

#include <ctime>
#include <iostream>

namespace Log
{

TStringList gErrorLog;
TString gLogFilename;
FILE *gpLogFile;

double gAppStartTime = CTimer::GlobalTime();

bool gInitialized = false;
TStringList gPreInitLogs;

bool InitLog(const TString& rkFilename)
{
    fopen_s(&gpLogFile, *rkFilename, "w");
    gLogFilename = rkFilename;

    if (!gpLogFile)
    {
        TString FileName = rkFilename.GetFileName(false);
        TString Extension = rkFilename.GetFileExtension();
        int Num = 0;

        while (!gpLogFile)
        {
            if (Num > 999) break;
            TString NewFilename = FileName + "_" + TString::FromInt32(Num, 0, 10) + "." + Extension;
            fopen_s(&gpLogFile, *NewFilename, "w");
            Num++;
        }

        if (!gpLogFile)
            return false;
    }

    // Print initial message to log
    time_t RawTime;
    time(&RawTime);

    tm pTimeInfo;
    localtime_s(&pTimeInfo, &RawTime);

    char Buffer[80];
    strftime(Buffer, 80, "%m/%d/%y %H:%M:%S", &pTimeInfo);

    fprintf(gpLogFile, "Opened log file at %s\n", Buffer);
    fflush(gpLogFile);

    // Print any messages that were attempted before we initialized
    if (!gPreInitLogs.empty())
    {
        for (auto it = gPreInitLogs.begin(); it != gPreInitLogs.end(); it++)
            Write(*it);
    }

    gInitialized = true;
    return true;
}

void Write(const TString& rkMessage)
{
    if (!gInitialized)
        gPreInitLogs.push_back(rkMessage);

    else if (gpLogFile)
    {
        double Time = CTimer::GlobalTime() - gAppStartTime;
        fprintf(gpLogFile, "[%08.3f] %s\n", Time, *rkMessage);
        fflush(gpLogFile);
    }

    std::cout << rkMessage << "\n";
}

void Error(const TString& rkMessage)
{
    TString FullMessage = "ERROR: " + rkMessage;
    Write(FullMessage);
    gErrorLog.push_back(FullMessage);
}

void Warning(const TString& rkMessage)
{
    TString FullMessage = "Warning: " + rkMessage;
    Write(FullMessage);
    gErrorLog.push_back(FullMessage);
}

void FileWrite(const TString& rkFilename, const TString& rkMessage)
{
    Write(rkFilename + " : " + rkMessage);
}

void FileWrite(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Write(rkFilename + " : " + TString::HexString(Offset, 0) + " - " + rkMessage);
}

void FileError(const TString& rkFilename, const TString& rkMessage)
{
    Error(rkFilename + " : " + rkMessage);
}

void FileError(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Error(rkFilename + " : " + TString::HexString(Offset, 0) + " - " + rkMessage);
}

void FileWarning(const TString& rkFilename, const TString& rkMessage)
{
    Warning(rkFilename + " : " + rkMessage);
}

void FileWarning(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Warning(rkFilename + " : " + TString::HexString(Offset, 0) + " - " + rkMessage);
}

const TStringList& GetErrorLog()
{
    return gErrorLog;
}

void ClearErrorLog()
{
    gErrorLog.clear();
}

}
