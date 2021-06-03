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

http::RequestParser::Result http::RequestParser::Parse(net::Socket* socket) {
  ClearState();

  do {
    for (int i = 0; i < kTimeout; i++) {
      pollfd fds;
      fds.fd = socket->GetFd();
      fds.events = POLLIN;

      int poll_ret = poll(&fds, 1, 1000);
      if (terminate_flag.load()) {
        result_.error = Error::kInterrupted;
        break;
      }

      if (poll_ret == -1) {
        log::Error("poll() failed: %s", strerror(errno));
        utils::Die();
      } else if (fds.revents & POLLIN) {
        std::string data = socket->Read(kBufferSize);

        if (data.size() == 0) {
          result_.error = Error::kNoData;
          break;
        }

        raw_request_ += data;

        HandleNewData(data);
        break;
      }
    }
  } while (state_ != State::kEnd);

  return result_;
}

http::RequestParser::Result http::RequestParser::Parse(const std::string& request) {
  ClearState();

  raw_request_ = request;
  HandleNewData(request);

  return result_;
}

void http::RequestParser::HandleNewData(std::string data) {
  for (int i = 0; i < data.size(); i++) {
    if (state_ == State::kEnd) {
      return;
    } else if (state_ == State::kContent) {
      result_.request.body += data.substr(i);
      if (result_.request.body.size() >= expected_content_size_) {
        state_ = State::kEnd;
      }
      return;
    } else {
      if (data[i] == '\r') {
        switch (state_) {
          case State::kHeader:        state_ = State::kHeaderCR; break;
          case State::kHeaderField:   state_ = State::kHeaderFieldCR; break;
          case State::kHeaderFieldCRLF:
          case State::kHeaderCRLF:    state_ = State::kCR; break;
          default:
            log::Warning("RequestParser is in an invalid state (char: '\\r', pos: %d, state: %d)", i, state_);
            result_.error = Error::kBadRequest;
        }
      } else if (data[i] == '\n') {
        switch (state_) {
          case State::kHeaderCR:      state_ = State::kHeaderCRLF; break;
          case State::kHeaderFieldCR: state_ = State::kHeaderFieldCRLF; break;
          case State::kCR:            state_ = State::kCRLF; break;
          default:
            log::Warning("RequestParser is in an invalid state (char: '\\n', pos: %d, state: %d)", i, state_);
            result_.error = Error::kBadRequest;
        }
        ParseCurrentLine();
      } else {
        if (state_ == State::kHeaderCRLF || state_ == State::kHeaderFieldCRLF) {
          state_ = State::kHeaderField;
        }
        current_line_.push_back(data[i]);
      }
    }
  }
}

void http::RequestParser::ParseCurrentLine() {
  if (HadError()) {
    return;
  }

  /* Check for request content */
  if (state_ == State::kCRLF) {
    if (HasContent()) {
      state_ = State::kContent;
      try {
        expected_content_size_ = std::stoi(result_.request.headers.at("Content-Length"));
      } catch (std::invalid_argument e) {
        state_ = State::kEnd;
        result_.error = Error::kBadRequest;
        log::Error("Content-Length is not a number");
        return;
      }
    } else {
      state_ = State::kEnd;
    }
    return;
  }

  /* Parse Header */
  if (state_ == State::kHeaderCRLF) {
    std::vector<std::string> tokens;
    int prev_token_idx = -1;
    for (int i = 0; i < current_line_.size(); i++) { // <
      if (current_line_[i] == ' ') {
        tokens.push_back(std::string(&current_line_[prev_token_idx+1], i-prev_token_idx-1));
        prev_token_idx = i;
      }
    }
    tokens.push_back(std::string(&current_line_[prev_token_idx+1], current_line_.size()-prev_token_idx-1));
  
    if (tokens.size() <= 3 && tokens.size() > 2) {
      result_.request.header.method = StringToHttpMethod(tokens[0]);
      if (tokens.size() == 3) {
        if (tokens[1].find("?") != std::string::npos) {
          int key_idx = tokens[1].find("?")+1;
          int eq_idx = key_idx;
          result_.request.header.url = tokens[1].substr(0, key_idx-1);
          for (int i = key_idx; i < tokens[1].size(); i++) {
            if (tokens[1][i] == '=') {
              eq_idx = i;
            }
            if (tokens[1][i] == '&') {
              result_.request.header.params[tokens[1].substr(key_idx, eq_idx-key_idx)] = tokens[1].substr(eq_idx+1, i-eq_idx-1);
              key_idx = i+1;
              eq_idx = key_idx;
            }
          }
          result_.request.header.params[tokens[1].substr(key_idx, eq_idx-key_idx)] = tokens[1].substr(eq_idx+1);
        } else {
          result_.request.header.url = tokens[1];
        }

        result_.request.header.http_version = tokens[2].substr(5);
      } else {
        result_.request.header.url = "/";
        result_.request.header.http_version = tokens[1].substr(5);
      }
    } else {
      result_.error = Error::kBadRequest;
      log::Warning("Invalid header: '%s'", current_line_.c_str());
      current_line_ = "";
      return;
    }
  } else if (state_ == State::kHeaderFieldCRLF) {
    std::string key = current_line_.substr(0, current_line_.find(":"));
    result_.request.headers[key] = current_line_.substr(current_line_.find(":")+2);
  } else {
    log::Warning("RequestParser is in an invalid state (line: '%s', state: %d)", current_line_.c_str(), state_);
    result_.error = Error::kBadRequest;
  }

  current_line_ = "";
}

void http::RequestParser::ClearState() {
  result_.error = Error::kNoError;
  result_.request = Request();
  raw_request_ = "";
  state_ = State::kHeader;
}

bool http::RequestParser::HadError() const {
  return result_.error != Error::kNoError;
}

bool http::RequestParser::HasContent() const {
  bool has_content_length_header = result_.request.headers.find("Content-Length") != result_.request.headers.end();
  if (has_content_length_header && std::stoi(result_.request.headers.at("Content-Length")) > 0) {
    return true;
  }
  return false;
}
