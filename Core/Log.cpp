#include <ctime>
#include <iostream>
#include <Common/TString.h>
#include <QMessageBox>

namespace Log
{

static const TString gskLogFilename = "primeworldeditor.log";
FILE *gpLogFile = fopen(*gskLogFilename, "w");

void Write(const TString& message)
{
    if (gpLogFile)
    {
        time_t RawTime;
        time(&RawTime);

        tm *pTimeInfo = localtime(&RawTime);
        char Buffer[80];
        strftime(Buffer, 80, "[%H:%M:%S]", pTimeInfo);

        fprintf(gpLogFile, "%s %s\n", Buffer, *message);
        fflush(gpLogFile);
    }
}

void Error(const TString& message)
{
    Write("ERROR: " + message);

#ifdef _DEBUG
        std::cout << "ERROR: " << message << "\n";
#endif
}

void Warning(const TString& message)
{
    Write("Warning: " + message);

#ifdef _DEBUG
    std::cout << "Warning: " << message << "\n";
#endif
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
