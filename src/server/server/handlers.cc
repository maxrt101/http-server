#include "mrt/server/server/handlers.h"

#include <unordered_map>

static std::unordered_map<int, http::Handler> handlers;

void http::SetHandler(int code, http::Handler handler) {
  handlers[code] = handler;
}

http::Handler http::GetHandler(int code) {
  return handlers[code];
}
