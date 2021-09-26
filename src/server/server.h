#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "mrt/server/server/http_server.h"
#include "mrt/server/config/server_config.h"
#include "mrt/server/version.h"

#include <string>

#define MRT_HTTP_VERSION_STRING "1.1"
#define MRT_HTTP_VERSION 11
#define MRT_HTTP_VERSION_MAJOR 1
#define MRT_HTTP_VERSION_MINOR 1
#define MTR_HTTP_VERSION_PATCH 0

namespace mrt {

class Server {
 public:
  Server();
  Server(config::HttpServerConfig conf);

  Server& port(int port);
  Server& root(const std::string& root);
  Server& defaultDocument(const std::string& doc);

  Server& addEndpoint(net::HttpServer::Endpoint endpoint);

  Server& run();

 private:
  net::HttpServer m_server;
};

} // namespace mrt

#endif
