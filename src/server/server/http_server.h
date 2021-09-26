#ifndef HTTP_SERVER_SERVER_SERVER_H_
#define HTTP_SERVER_SERVER_SERVER_H_

#include <string>
#include <functional>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mrt/threads/pool.h"
#include "mrt/threads/task.h"

#include "mrt/server/sockets/bsd_socket.h"
#include "mrt/server/sockets/socket.h"
#include "mrt/server/http/response.h"
#include "mrt/server/http/request.h"
#include "mrt/server/config/server_config.h"

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
  struct TaskParams {
    HttpServer* server;
    net::Socket* client;
  };

 public:
  HttpServer();
  HttpServer(config::HttpServerConfig conf);
  ~HttpServer();

  void init();
  void init(config::HttpServerConfig conf);
  config::HttpServerConfig& getConfig();
  void run();
  void stop();

  void addEndpoint(Endpoint endpoint);

 private:
  static void dealWithClientTask(void* args);
  void dealWithClient(net::Socket* client);

 private:
  net::BsdSocket m_socket;
  mrt::threads::ThreadPool<mrt::threads::Task<TaskParams*>> m_pool;
  config::HttpServerConfig m_conf;
  std::unordered_map<std::string, std::unordered_map<int, Endpoint>> m_endpoints;
};
} // namespace server

#endif
