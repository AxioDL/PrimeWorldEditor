#include <Common/TString.h>
#include <ctime>
#include <iostream>

namespace Log
{

static const TString gskLogFilename = "primeworldeditor.log";

#pragma warning(push)
#pragma warning(disable: 4996) // Can't use fopen_s here without creating a separate init function for the log
FILE *gpLogFile = fopen(*gskLogFilename, "w");
#pragma warning(pop)

void Write(const TString& message)
{
    if (gpLogFile)
    {
        time_t RawTime;
        time(&RawTime);

        tm pTimeInfo;
        localtime_s(&pTimeInfo, &RawTime);

        char Buffer[80];
        strftime(Buffer, 80, "[%H:%M:%S]", &pTimeInfo);

        fprintf(gpLogFile, "%s %s\n", Buffer, *message);
        fflush(gpLogFile);
    }
}

void Error(const TString& message)
{
    Write("ERROR: " + message);
    std::cout << "ERROR: " << message << "\n";
}

void Warning(const TString& message)
{
    Write("Warning: " + message);
    std::cout << "Warning: " << message << "\n";
}

void FileWrite(const TString& filename, const TString& message)
{
    Write(filename + " : " + message);
}

void FileWrite(const TString& filename, unsigned long offset, const TString& message)
{
    Write(filename + " : " + TString::HexString(offset) + " - " + message);
}

void FileError(const TString& filename, const TString& message)
{
    Error(filename + " : " + message);
}

void FileError(const TString& filename, unsigned long offset, const TString& message)
{
    Error(filename + " : " + TString::HexString(offset) + " - " + message);
}

void FileWarning(const TString& filename, const TString& message)
{
    Warning(filename + " : " + message);
}

void FileWarning(const TString& filename, unsigned long offset, const TString& message)
{
    Warning(filename + " : " + TString::HexString(offset) + " - " + message);
}

}
