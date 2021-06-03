#ifndef SERVER_HTTP_PARSER_H_
#define SERVER_HTTP_PARSER_H_

#include "server/sockets/socket.h"
#include "server/http/request.h"

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
    kHeader,
    kHeaderCR,
    kHeaderCRLF,
    kHeaderField,
    kHeaderFieldCR,
    kHeaderFieldCRLF,
    kCR,
    kCRLF,
    kContent,
    kEnd,
  };

 public:
  Result Parse(net::Socket* socket);
  Result Parse(const std::string& request);

 private:
  void ClearState();
  bool HadError() const;
  bool HasContent() const;
  void HandleNewData(std::string data);
  void ParseCurrentLine();

 private:
  Result result_;
  State state_;
  int expected_content_size_ = 0;
  std::string raw_request_;
  std::string current_line_;
};

} // namepsace http

#endif