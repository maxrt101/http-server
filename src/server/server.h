#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "mrt/server/server/http_server.h"
#include "mrt/server/config/server_config.h"

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
