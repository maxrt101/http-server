#include "server/utils/die.h"

#include "server/debug/log.h"

void utils::Die(int exit_code) {
  if (log::IsLogActive()) {
    log::EndLog();
  }
  exit(exit_code);
}