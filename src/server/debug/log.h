#ifndef SERVER_DEBUG_LOG_H_
#define SERVER_DEBUG_LOG_H_

#include <string>

namespace log {

void startLog(const std::string& file_name);
void endLog();
bool isLogActive();

void error(const char* fmt, ...);
void warning(const char* fmt, ...);
void info(const char* fmt, ...);
void debug(const char* fmt, ...);

} // namespace log

#endif
