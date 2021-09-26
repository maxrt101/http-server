#include "mrt/args/args.h"
#include "server/server.h"
#include "server/debug/log.h"

#include <iostream>

int main(int argc, const char ** argv) {
  auto args = mrt::args::Parser("Lab3 Server", {
    {"version", 'F', {"-v", "--version"}, "Shows version"},
    {"port", 'V', {"-p", "--port"}, "Server Port (default '80')"},
    {"root", 'V', {"-r", "--root"}, "Root Folder"},
    {"log", 'V', {"-l", "--log"}, "Log file (default 'server.log')"},
    {"api", 'V', {"-a", "--api"}, "Rest API Example path (default '/api')"}
  }).parse(argc, argv);

  if (args.exists("version")) {
    std::cout << "mrthttp v" << MRT_HTTP_VERSION_STRING << " by maxrt101" << std::endl;
    return 0;
  }

  int port = 80;
  try {
    port = std::stoi(args.getFirstOr("port", "80"));
  } catch (std::exception& e) {
    std::cout << "Error: invalid port" << std::endl;
    return 1;
  }
  
  log::startLog(args.getFirstOr("log", "server.log"));

  mrt::Server server;

  server.addEndpoint({args.getFirstOr("api", "/api"), http::Method::GET, [](auto request) {
    return http::Response(http::OK).setContent("text/plain", "Hello, HTTP Server!");
  }});

  server.addEndpoint({args.getFirstOr("api", "/api"), http::Method::POST, [](auto request) {
    std::string body;
    for (auto& p : request.header.params) {
      body += p.first + " = " + p.second + "\n";
    }
    return http::Response(http::OK).setContent("text/plain", body);
  }});

  server.port(port).root(args.getFirstOr("root", ".")).run();

  return 0;
}
