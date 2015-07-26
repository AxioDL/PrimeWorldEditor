#include <ctime>
#include <iostream>
#include <string>
#include <QMessageBox>
#include <Common/StringUtil.h>

namespace Log
{

static const std::string gskLogFilename = "primeworldeditor.log";
FILE *gpLogFile = fopen(gskLogFilename.c_str(), "w");

void Write(const std::string& message)
{

    if (gpLogFile)
    {
        time_t RawTime;
        time(&RawTime);

        tm *pTimeInfo = localtime(&RawTime);
        char Buffer[80];
        strftime(Buffer, 80, "[%H:%M:%S]", pTimeInfo);

        fprintf(gpLogFile, "%s %s\n", Buffer, message.c_str());
        fflush(gpLogFile);
    }
}

void Error(const std::string& message)
{
    Write("ERROR: " + message);

#ifdef _DEBUG
        std::cout << "ERROR: " << message << "\n";
#endif
}

void Warning(const std::string& message)
{
    Write("Warning: " + message);

#ifdef _DEBUG
    std::cout << "Warning: " << message << "\n";
#endif
}

void FileWrite(const std::string& filename, const std::string& message)
{
    Write(filename + " : " + message);
}

void FileWrite(const std::string& filename, unsigned long offset, const std::string& message)
{
    Write(filename + " : " + StringUtil::ToHexString(offset) + " - " + message);
}

void FileError(const std::string& filename, const std::string& message)
{
    Error(filename + " : " + message);
}

void FileError(const std::string &filename, unsigned long offset, const std::string &message)
{
    Error(filename + " : " + StringUtil::ToHexString(offset) + " - " + message);
}

void FileWarning(const std::string& filename, const std::string& message)
{
    Warning(filename + " : " + message);
}

void FileWarning(const std::string& filename, unsigned long offset, const std::string& message)
{
    Warning(filename + " : " + StringUtil::ToHexString(offset) + " - " + message);
}

}
