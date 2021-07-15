#ifndef SERVER_HTTP_REQUEST_H_
#define SERVER_HTTP_REQUEST_H_

#include <string>
#include <map>

namespace http {

enum class Method {
  NONE = 0,
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH,
};

std::string GetMethodName(Method method);

struct RequestHeader {
 public:
  Method method;
  std::string url;
  std::map<std::string, std::string> params;
  std::string http_version;

 public:
  std::string getString() const;
};

struct Request {
 public:
  RequestHeader header;
  std::map<std::string, std::string> headers;
  std::string body;

 public:
  std::string getString() const;
};

} // namespace http

#endif