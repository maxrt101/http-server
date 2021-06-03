#include "server/debug/log.h"

#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>

static FILE* log_file;
static std::mutex output_mutex;

constexpr auto kGreen = "\033[1;32m";
constexpr auto kBlue = "\033[1;34m";
constexpr auto kYellow = "\033[1;33m";
constexpr auto kRed = "\033[1;31m";

static std::string GetDateTimeString() {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  char buffer[26] {0};
  //std::string s(19, '\0');
  std::strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  return std::string(buffer);
}

static void Log(const std::string& tag, const std::string& color, bool to_stderr, const char* fmt, va_list args1, va_list args2) {
  const std::lock_guard<std::mutex> output_lock(output_mutex);

  std::string thread_id = (std::stringstream() << std::this_thread::get_id()).str();

  FILE* out = to_stderr ? stderr : stdout;
  fprintf(out, "%s", color.c_str());
  fprintf(out, "[%s] [%s] [%s] ", GetDateTimeString().c_str(), thread_id.c_str(), tag.c_str());
  vfprintf(out, fmt, args1);
  fprintf(out, "\033[0m\n");
  fflush(out);

  if (log_file) {
    fprintf(log_file, "[%s] [%s] [%s] ", GetDateTimeString().c_str(), thread_id.c_str(), tag.c_str());
    vfprintf(log_file, fmt, args2);
    fprintf(log_file, "\n");
    fflush(log_file);
  }
}

void log::StartLog(const std::string& file_name) {
  const std::lock_guard<std::mutex> output_lock(output_mutex);
  log_file = fopen(file_name.c_str(), "a");
  if (!log_file) {
    Log("ERROR", "", true, "Cannot open log file", NULL, NULL);
    // utils::Die();
  }
}

void log::EndLog() {
  fclose(log_file);
  log_file = NULL;
}

bool log::IsLogActive() {
  return log_file;
}

void log::Error(const char* fmt, ...) {
  va_list args1, args2;
  va_start(args1, fmt);
  va_start(args2, fmt);
  Log("ERROR", kRed, true, fmt, args1, args2);
  va_end(args1);
  va_end(args2);
}

void log::Warning(const char* fmt, ...) {
  va_list args1, args2;
  va_start(args1, fmt);
  va_start(args2, fmt);
  Log("WARNING", kYellow, true, fmt, args1, args2);
  va_end(args1);
  va_end(args2);
}

void log::Info(const char* fmt, ...) {
  va_list args1, args2;
  va_start(args1, fmt);
  va_start(args2, fmt);
  Log("INFO", "", false, fmt, args1,args2);
  va_end(args1);
  va_end(args2);
}

void log::Debug(const char* fmt, ...) {
  va_list args1, args2;
  va_start(args1, fmt);
  va_start(args2, fmt);
  Log("DEBUG", kBlue, false, fmt, args1, args2);
  va_end(args1);
  va_end(args2);
}
