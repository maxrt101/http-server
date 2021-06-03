#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "server/server/http_server.h"
#include "server/config/server_config.h"

namespace mrt {

class Server {
 public:
  Server();
  Server(config::HttpServerCofig conf);

  Server& AddEndpoint(net::HttpServer::Endpoint endpoint);

  Server& Run();

 private:
  net::HttpServer server_;
};

} // namespace mrt

#endif