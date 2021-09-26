#include "server.h"

mrt::Server::Server() {
  m_server.init();
}

mrt::Server::Server(config::HttpServerConfig conf) {
  m_server.init(conf);
}

mrt::Server& mrt::Server::port(int port) {
  m_server.getConfig().port = port;
  return *this;
}

mrt::Server& mrt::Server::root(const std::string& root) {
  m_server.getConfig().root_folder = root;
  return *this;
}

mrt::Server& mrt::Server::defaultDocument(const std::string& doc) {
  m_server.getConfig().default_doc = doc;
  return *this;
}

mrt::Server& mrt::Server::addEndpoint(net::HttpServer::Endpoint endpoint) {
  m_server.addEndpoint(endpoint);
  return *this;
}

mrt::Server& mrt::Server::run() {
  m_server.run();
  return *this;
}
