#include "server.h"

mrt::Server::Server() {
  m_server.init();
}

mrt::Server::Server(config::HttpServerCofig conf) {
  m_server.init(conf);
}

mrt::Server& mrt::Server::addEndpoint(net::HttpServer::Endpoint endpoint) {
  m_server.addEndpoint(endpoint);
  return *this;
}

mrt::Server& mrt::Server::run() {
  m_server.run();
  return *this;
}