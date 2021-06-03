#include "mrt/args/args.h"
#include "server/server.h"
#include "server/debug/log.h"

#include <iostream>

int main(int argc, const char ** argv) {
  log::StartLog("server.log");

  mrt::Server server;

  server.AddEndpoint({"/api", http::Method::GET, [](auto request) {
    return http::Response(http::OK).SetContent("text/plain", "Hello, HTTP Server!");
  }});

  server.AddEndpoint({"/api", http::Method::POST, [](auto request) {
    std::string body;
    for (auto& p : request.header.params) {
      body += p.first + " = " + p.second + "\n";
    }
    return http::Response(http::OK).SetContent("text/plain", body);
  }});

  server.Run();

  return 0;
}