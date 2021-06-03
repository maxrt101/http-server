#ifndef SERVER_DEBUG_LOG_H_
#define SERVER_DEBUG_LOG_H_

#include <string>

namespace log {

void StartLog(const std::string& file_name);
void EndLog();
bool IsLogActive();

void Error(const char* fmt, ...);
void Warning(const char* fmt, ...);
void Info(const char* fmt, ...);
void Debug(const char* fmt, ...);

} // namespace log

#endif