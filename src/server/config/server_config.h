#ifndef SERVER_CONFIG_SERVER_CONFIG_H_
#define SERVER_CONFIG_SERVER_CONFIG_H_

namespace config {
struct HttpServerCofig {
  int port = 80;
  int keep_alive_max = 5;
  int keep_alive_timeout = 120;
  // bool chunked_transfer = false;
  std::string default_doc = "index.html";
  std::string root_folder;

  // void LoadFromConfigFile()
};
} // namespace config

#endif