#ifndef INFO
#define INFO

#include <string>

namespace Log
{

void Write(const std::string& message);
void Error(const std::string& message);
void Warning(const std::string& message);
void FileWrite(const std::string& filename, const std::string& message);
void FileWrite(const std::string& filename, unsigned long offset, const std::string& message);
void FileError(const std::string& filename, const std::string& message);
void FileError(const std::string& filename, unsigned long offset, const std::string& message);
void FileWarning(const std::string& filename, const std::string& message);
void FileWarning(const std::string& filename, unsigned long offset, const std::string& message);

}

#endif // INFO

