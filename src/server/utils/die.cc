#include "mrt/server/utils/die.h"

#include "mrt/server/debug/log.h"

void utils::Die(int exit_code) {
  if (log::isLogActive()) {
    log::endLog();
  }
  exit(exit_code);
}
