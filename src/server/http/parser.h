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
  Result parse(net::Socket* socket);
  Result parse(const std::string& request);

 private:
  void clearState();
  bool hadError() const;
  bool hasContent() const;
  void handleNewData(std::string data);
  void parseCurrentLine();

 private:
  Result m_result;
  State m_state;
  int m_expected_content_size = 0;
  std::string m_raw_request;
  std::string m_current_line;
};

} // namepsace http

#endif
