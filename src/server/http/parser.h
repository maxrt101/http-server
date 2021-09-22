#ifndef SERVER_HTTP_PARSER_H_
#define SERVER_HTTP_PARSER_H_

#include "mrt/server/sockets/socket.h"
#include "mrt/server/http/request.h"

#include <vector>

namespace http {

Method StringToHttpMethod(const std::string& str);

class RequestParser {
 public:
  enum class Error {
    kNoError = 0,
    kBadRequest,
    kRequestTimeout,
    kNoData,
    kInterrupted,
  };

  struct Result {
    Request request;
    Error error;

    operator bool() const;
  };

 private:
  enum class State {
    kHeaderMethod,
    kHeaderUrl,
    kHeaderHttpVersion,
    kHeaderCRLF,
    kHeaderField,
    kHeaderFieldCRLF,
    kCRLF,
    kContent,
    kEnd,
  };

 public:
  Result parse(net::Socket* socket);
  Result parse(const std::string& request);

  void parseUrl(const std::string url);

 private:
  void clearState();
  bool hadError() const;
  bool hasContent() const;
  void handleNewData(std::string data);

 private:
  Result m_result;
  State m_state;

  int m_expected_content_size = 0;
  std::string m_raw_request;

  std::string m_method;
  std::string m_param_key;
  std::string m_param_value;
  bool m_param_is_key = true;
};

} // namepsace http

#endif
