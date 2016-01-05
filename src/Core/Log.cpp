#include <Common/TString.h>
#include <ctime>
#include <iostream>

namespace Log
{

TStringList ErrorLog;
static const TString gskLogFilename = "primeworldeditor.log";

#pragma warning(push)
#pragma warning(disable: 4996) // Can't use fopen_s here without creating a separate init function for the log
FILE *gpLogFile = fopen(*gskLogFilename, "w");
#pragma warning(pop)

void Write(const TString& rkMessage)
{
    if (gpLogFile)
    {
        time_t RawTime;
        time(&RawTime);

        tm pTimeInfo;
        localtime_s(&pTimeInfo, &RawTime);

        char Buffer[80];
        strftime(Buffer, 80, "[%H:%M:%S]", &pTimeInfo);

        fprintf(gpLogFile, "%s %s\n", Buffer, *rkMessage);
        fflush(gpLogFile);
    }
}

void Error(const TString& rkMessage)
{
    TString FullMessage = "ERROR: " + rkMessage;
    Write(FullMessage);
    ErrorLog.push_back(FullMessage);
    std::cout << FullMessage << "\n";
}

void Warning(const TString& rkMessage)
{
    TString FullMessage = "Warning: " + rkMessage;
    Write(FullMessage);
    ErrorLog.push_back(FullMessage);
    std::cout << FullMessage << "\n";
}

void FileWrite(const TString& rkFilename, const TString& rkMessage)
{
    Write(rkFilename + " : " + rkMessage);
}

void FileWrite(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Write(rkFilename + " : " + TString::HexString(Offset) + " - " + rkMessage);
}

void FileError(const TString& rkFilename, const TString& rkMessage)
{
    Error(rkFilename + " : " + rkMessage);
}

void FileError(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Error(rkFilename + " : " + TString::HexString(Offset) + " - " + rkMessage);
}

void FileWarning(const TString& rkFilename, const TString& rkMessage)
{
    Warning(rkFilename + " : " + rkMessage);
}

void FileWarning(const TString& rkFilename, u32 Offset, const TString& rkMessage)
{
    Warning(rkFilename + " : " + TString::HexString(Offset) + " - " + rkMessage);
}

const TStringList& GetErrorLog()
{
    return ErrorLog;
}

void ClearErrorLog()
{
    ErrorLog.clear();
}

}
