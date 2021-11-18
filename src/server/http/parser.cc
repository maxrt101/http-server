#include "mrt/server/http/parser.h"

#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "mrt/server/debug/log.h"
#include "mrt/server/utils/die.h"
#include "mrt/container_utils.h"

extern std::atomic<bool> g_terminate_flag;

static constexpr int kBufferSize = 1024;
static constexpr int kMaxWaitTime = 2;
static constexpr int kTimeout = 1; //30;

http::Method http::StringToHttpMethod(const std::string& str) {
  if (str == "GET") {             return Method::GET;
  } else if (str == "HEAD") {     return Method::HEAD;
  } else if (str == "POST") {     return Method::POST;
  } else if (str == "PUT") {      return Method::PUT;
  } else if (str == "DELETE") {   return Method::DELETE;
  } else if (str == "CONNECT") {  return Method::CONNECT;
  } else if (str == "OPTIONS") {  return Method::OPTIONS;
  } else if (str == "TRACE") {    return Method::TRACE;
  } else if (str == "PATCH") {    return Method::PATCH;
  } else {                        return Method::NONE;
  }
}

http::RequestParser::Result::operator bool() const {
  return error == Error::kNoError;
}

http::RequestParser::Result http::RequestParser::parse(net::Socket* socket) {
  clearState();

  for (int i = 0; i < kTimeout; i++) {
    pollfd fds;
    fds.fd = socket->getFd();
    fds.events = POLLIN;

    int poll_ret = poll(&fds, 1, 1000);
    if (g_terminate_flag.load()) {
      m_result.error = Error::kInterrupted;
      break;
    }

    if (poll_ret == -1) {
      log::error("poll() failed: %s", strerror(errno));
      utils::Die();
    } else if (poll_ret == 0) {
      // timeout
    } else if (fds.revents & POLLIN) {
      std::string data = socket->read(kBufferSize);

      if (data.size() == 0) {
        m_result.error = Error::kNoData;
        break;
      }

      m_raw_request += data;

      log::debug("New Data{%s}", data.c_str());

      handleNewData(data);
      break;
    }
  }

  return m_result;
}

http::RequestParser::Result http::RequestParser::parse(const std::string& request) {
  clearState();

  m_raw_request = request;
  handleNewData(request);

  return m_result;
}

void http::RequestParser::handleNewData(std::string data) {
  for (int i = 0; i < data.size(); i++) {
    if (m_state == State::kHeaderMethod) {
      if (data[i] == ' ') {
        m_result.request.header.method = StringToHttpMethod(m_method);
        m_state = State::kHeaderUrl;
      } else {
        m_method += data[i];
      }
    } else if (m_state == State::kHeaderUrl) {
      if (data[i] == ' ') {
        parseUrl(m_result.request.header.url);
        m_state = State::kHeaderHttpVersion;
      } else {
        m_result.request.header.url += data[i];
      }
    } else if (m_state == State::kHeaderHttpVersion) {
      if (data[i] == '\r') {
      } else if (data[i] == '\n') {
        m_state = State::kHeaderCRLF;
      } else {
        m_result.request.header.http_version += data[i];
      }
    } else if (m_state == State::kHeaderCRLF) {
      if (data[i] == '\r') {
      } else if (data[i] == '\n') {
        m_state = State::kCRLF;
      } else {
        m_state = State::kHeaderField;
      }
    } else if (m_state == State::kHeaderField) {
      if (data[i] == ':') {
        m_param_is_key = false;
        i++;
      } else if (data[i] == '\r') {
      } else if (data[i] == '\n') {
        m_result.request.headers[m_param_key] = m_param_value;
        m_param_key = "";
        m_param_value = "";
        m_param_is_key = true;
        m_state = State::kHeaderFieldCRLF;
      } else {
        if (m_param_is_key) {
          m_param_key += data[i];
        } else {
          m_param_value += data[i];
        }
      }
    } else if (m_state == State::kHeaderFieldCRLF) {
      if (data[i] == '\r') {
      } else if (data[i] == '\n') {
        if (mrt::hasKey(m_result.request.headers, std::string("Content-Length"))) {
          m_expected_content_size = std::stoi(m_result.request.headers.at("Content-Length"));
        }
        if (hasContent()) {
          m_state = State::kContent;
        } else {
          m_state = State::kEnd;
        }
      } else {
        m_state = State::kHeaderField;
        i--;
      }
    } else if (m_state == State::kCRLF) {
      // ?
    } else if (m_state == State::kContent) {
      if (m_result.request.body.size() < m_expected_content_size) {
        m_result.request.body += data[i];
      } else {
        m_state = State::kEnd;
      }
    } else if (m_state == State::kEnd) {
      return;
    } else {
      log::warning("RequestParser is in an invalid state (state: %d)", m_state);
      m_result.error = Error::kBadRequest;
    }

  }
}

void http::RequestParser::parseUrl(const std::string url) {
  if (!url.size()) {
    m_result.request.header.url = "";
    return;
  }

  if (url.find("?") != std::string::npos) {
    int key_idx = url.find("?")+1;
    int eq_idx = key_idx;
    m_result.request.header.url = url.substr(0, key_idx-1);
    for (int i = key_idx; i < url.size(); i++) {
      if (url[i] == '=') {
        eq_idx = i;
      }
      if (url[i] == '&') {
        m_result.request.header.params[url.substr(key_idx, eq_idx-key_idx)] = url.substr(eq_idx+1, i-eq_idx-1);
        key_idx = i+1;
        eq_idx = key_idx;
      }
    }
    m_result.request.header.params[url.substr(key_idx, eq_idx-key_idx)] = url.substr(eq_idx+1);
  } else {
    m_result.request.header.url = url;
  }
}

void http::RequestParser::clearState() {
  m_result.error = Error::kNoError;
  m_result.request = Request();
  m_raw_request = "";
  m_state = State::kHeaderMethod;
}

bool http::RequestParser::hadError() const {
  return m_result.error != Error::kNoError;
}

bool http::RequestParser::hasContent() const {
  if (mrt::hasKey(m_result.request.headers, std::string("Content-Length")) && m_expected_content_size > 0) {
    return true;
  }
  return false;
}
