#include "server/http/parser.h"

#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "server/debug/log.h"
#include "server/utils/die.h"

extern std::atomic<bool> terminate_flag;

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

  do {
    for (int i = 0; i < kTimeout; i++) {
      pollfd fds;
      fds.fd = socket->getFd();
      fds.events = POLLIN;

      int poll_ret = poll(&fds, 1, 1000);
      if (terminate_flag.load()) {
        m_result.error = Error::kInterrupted;
        break;
      }

      if (poll_ret == -1) {
        log::error("poll() failed: %s", strerror(errno));
        utils::Die();
      } else if (fds.revents & POLLIN) {
        std::string data = socket->read(kBufferSize);

        if (data.size() == 0) {
          m_result.error = Error::kNoData;
          break;
        }

        m_raw_request += data;

        handleNewData(data);
        break;
      }
    }
  } while (m_state != State::kEnd);

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
    if (m_state == State::kEnd) {
      return;
    } else if (m_state == State::kContent) {
      m_result.request.body += data.substr(i);
      if (m_result.request.body.size() >= m_expected_content_size) {
        m_state = State::kEnd;
      }
      return;
    } else {
      if (data[i] == '\r') {
        switch (m_state) {
          case State::kHeader:        m_state = State::kHeaderCR; break;
          case State::kHeaderField:   m_state = State::kHeaderFieldCR; break;
          case State::kHeaderFieldCRLF:
          case State::kHeaderCRLF:    m_state = State::kCR; break;
          default:
            log::warning("RequestParser is in an invalid state (char: '\\r', pos: %d, state: %d)", i, m_state);
            m_result.error = Error::kBadRequest;
        }
      } else if (data[i] == '\n') {
        switch (m_state) {
          case State::kHeaderCR:      m_state = State::kHeaderCRLF; break;
          case State::kHeaderFieldCR: m_state = State::kHeaderFieldCRLF; break;
          case State::kCR:            m_state = State::kCRLF; break;
          default:
            log::warning("RequestParser is in an invalid state (char: '\\n', pos: %d, state: %d)", i, m_state);
            m_result.error = Error::kBadRequest;
        }
        parseCurrentLine();
      } else {
        if (m_state == State::kHeaderCRLF || m_state == State::kHeaderFieldCRLF) {
          m_state = State::kHeaderField;
        }
        m_current_line.push_back(data[i]);
      }
    }
  }
}

void http::RequestParser::parseCurrentLine() {
  if (hadError()) {
    return;
  }

  /* Check for request content */
  if (m_state == State::kCRLF) {
    if (hasContent()) {
      m_state = State::kContent;
      try {
        m_expected_content_size = std::stoi(m_result.request.headers.at("Content-Length"));
      } catch (std::invalid_argument e) {
        m_state = State::kEnd;
        m_result.error = Error::kBadRequest;
        log::error("Content-Length is not a number");
        return;
      }
    } else {
      m_state = State::kEnd;
    }
    return;
  }

  /* Parse Header */
  if (m_state == State::kHeaderCRLF) {
    std::vector<std::string> tokens;
    int prev_token_idx = -1;
    for (int i = 0; i < m_current_line.size(); i++) { // <
      if (m_current_line[i] == ' ') {
        tokens.push_back(std::string(&m_current_line[prev_token_idx+1], i-prev_token_idx-1));
        prev_token_idx = i;
      }
    }
    tokens.push_back(std::string(&m_current_line[prev_token_idx+1], m_current_line.size()-prev_token_idx-1));
  
    if (tokens.size() <= 3 && tokens.size() > 2) {
      m_result.request.header.method = StringToHttpMethod(tokens[0]);
      if (tokens.size() == 3) {
        if (tokens[1].find("?") != std::string::npos) {
          int key_idx = tokens[1].find("?")+1;
          int eq_idx = key_idx;
          m_result.request.header.url = tokens[1].substr(0, key_idx-1);
          for (int i = key_idx; i < tokens[1].size(); i++) {
            if (tokens[1][i] == '=') {
              eq_idx = i;
            }
            if (tokens[1][i] == '&') {
              m_result.request.header.params[tokens[1].substr(key_idx, eq_idx-key_idx)] = tokens[1].substr(eq_idx+1, i-eq_idx-1);
              key_idx = i+1;
              eq_idx = key_idx;
            }
          }
          m_result.request.header.params[tokens[1].substr(key_idx, eq_idx-key_idx)] = tokens[1].substr(eq_idx+1);
        } else {
          m_result.request.header.url = tokens[1];
        }

        m_result.request.header.http_version = tokens[2].substr(5);
      } else {
        m_result.request.header.url = "/";
        m_result.request.header.http_version = tokens[1].substr(5);
      }
    } else {
      m_result.error = Error::kBadRequest;
      log::warning("Invalid header: '%s'", m_current_line.c_str());
      m_current_line = "";
      return;
    }
  } else if (m_state == State::kHeaderFieldCRLF) {
    std::string key = m_current_line.substr(0, m_current_line.find(":"));
    m_result.request.headers[key] = m_current_line.substr(m_current_line.find(":")+2);
  } else {
    log::warning("RequestParser is in an invalid state (line: '%s', state: %d)", m_current_line.c_str(), m_state);
    m_result.error = Error::kBadRequest;
  }

  m_current_line = "";
}

void http::RequestParser::clearState() {
  m_result.error = Error::kNoError;
  m_result.request = Request();
  m_raw_request = "";
  m_state = State::kHeader;
}

bool http::RequestParser::hadError() const {
  return m_result.error != Error::kNoError;
}

bool http::RequestParser::hasContent() const {
  bool has_content_length_header = m_result.request.headers.find("Content-Length") != m_result.request.headers.end();
  if (has_content_length_header && std::stoi(m_result.request.headers.at("Content-Length")) > 0) {
    return true;
  }
  return false;
}
