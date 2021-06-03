#include "server/server.h"

mrt::Server::Server() {
  server_.Init();
}

mrt::Server::Server(config::HttpServerCofig conf) {
  server_.Init(conf);
}

mrt::Server& mrt::Server::AddEndpoint(net::HttpServer::Endpoint endpoint) {
  server_.AddEndpoint(endpoint);
  return *this;
}

mrt::Server& mrt::Server::Run() {
  server_.Run();
  return *this;
}