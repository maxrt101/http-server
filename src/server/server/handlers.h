#ifndef SERVER_SERVER_HANDLERS_H_
#define SERVER_SERVER_HANDLERS_H_

#include <functional>

#include "mrt/server/http/response.h"

namespace http {

using Handler = std::function<Response(const Response&)>;

void SetHandler(int code, Handler handler = nullptr);
Handler GetHandler(int code);

} // namespace http

#endif
