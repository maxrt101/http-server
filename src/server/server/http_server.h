#ifndef HTTP_SERVER_SERVER_SERVER_H_
#define HTTP_SERVER_SERVER_SERVER_H_

#include <string>
#include <functional>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mrt/threads/pool.h"

#include "server/sockets/bsd_socket.h"
#include "server/sockets/socket.h"
#include "server/http/response.h"
#include "server/http/request.h"
#include "server/config/server_config.h"

namespace net {
class HttpServer {
 public:
  using endpoint_funciton = std::function<http::Response(const http::Request&)>;

 public:
  struct Endpoint {
    std::string url;
    http::Method method;
    endpoint_funciton function;
  };

 private:
  struct JobParams {
    HttpServer* server;
    net::Socket* client;
  };

 public:
  HttpServer();
  HttpServer(config::HttpServerCofig conf);
  ~HttpServer();

  void Init();
  void Init(config::HttpServerCofig conf);
  void Run();
  void Stop();

  void AddEndpoint(Endpoint endpoint);

 private:
  static void DealWithClientJob(void* args);
  void DealWithClient(net::Socket* client);

 private:
  net::BsdSocket socket_;
  mrt::threads::ThreadPool pool_;
  config::HttpServerCofig conf_;
  std::unordered_map<std::string, std::unordered_map<int, Endpoint>> endpoints_;
};
} // namespace server

#endif
