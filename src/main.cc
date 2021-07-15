#include "mrt/args/args.h"
#include "server/server.h"
#include "server/debug/log.h"

#include <iostream>

int main(int argc, const char ** argv) {
  log::startLog("server.log");

  mrt::Server server;

  server.addEndpoint({"/api", http::Method::GET, [](auto request) {
    return http::Response(http::OK).setContent("text/plain", "Hello, HTTP Server!");
  }});

  server.addEndpoint({"/api", http::Method::POST, [](auto request) {
    std::string body;
    for (auto& p : request.header.params) {
      body += p.first + " = " + p.second + "\n";
    }
    return http::Response(http::OK).setContent("text/plain", body);
  }});

  server.run();

  return 0;
}