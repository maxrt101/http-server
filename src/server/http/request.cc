#include "server/http/request.h"

#include <iostream>
#include <sstream>

static constexpr char kCRLF[] = "\r\n";

std::string http::GetMethodName(http::Method method) {
  switch (method) {
    case Method::GET:     return "GET";
    case Method::HEAD:    return "HEAD";
    case Method::POST:    return "POST";
    case Method::PUT:     return "PUT";
    case Method::DELETE:  return "DELETE";
    case Method::CONNECT: return "CONNECT";
    case Method::OPTIONS: return "OPTIONS";
    case Method::TRACE:   return "TRACE";
    case Method::PATCH:   return "PATCH";
    default:              return "?";
  }
}

std::string http::RequestHeader::GetString() const {
  std::stringstream ss;
  ss << GetMethodName(method) << " " << url;
  if (!params.empty()) {
    ss << "?";
    int i = 0;
    for (auto& p : params) {
      ss << p.first << "=" << p.second;
      if (i+1 != params.size()) {
        ss << "&";
      }
      i++;
    }
  }
  ss << " HTTP/" << http_version;
  return ss.str();
}

std::string http::Request::GetString() const {
  std::stringstream ss;
  ss << header.GetString() << kCRLF;
  for (auto& header_field : headers) {
    ss << header_field.first << ": " << header_field.second << kCRLF;
  }
  ss << kCRLF << body;
  return ss.str();
}