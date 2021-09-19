# HTTP Server
Simple HTTP Server implementation in C++.  
Supports flask-like API for creating endpoints.  

### How To Build:
Dependencies: `git`, `gcc`/`clang`, `make`, binutils(`ar`)

 - Clone repo
 - `cd http-server`
 - `make PREFIX=/path/to/install/dir`, set `PREFIX` to desired install location. If no prefix is specified, `build` folder will be created

After install `libhttpserver.a` and `libmrt.a` will be present inside `PREFIX/lib` along with all required headers (in `PREFIX/include`)

To embed http-server into your project, you can just include it as a git submodule, and run `make -C http-server` inside your projects `Makefile`

### Example:
```c++

mrt::Server server;

server.addEndpoint({"/api", http::Method::GET, [](auto request) {
  return http::Response(http::OK).setContent("text/plain", "Hello, World!");
}});

server.addEndpoint({"/api", http::Method::POST, [](auto request) {
  std::string body;
  for (auto& p : request.header.params) {
    body += p.first + " = " + p.second + "\n";
  }
  return http::Response(http::OK).setContent("text/plain", body);
}});

server.run();

```

