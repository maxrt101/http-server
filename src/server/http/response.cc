#include "server/http/response.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

#include "server/server/handlers.h"
#include "server/http/request.h"
#include "server/debug/log.h"

static constexpr char kCRLF[] = "\r\n";

static bool IsPathValid(const std::string& path) {
  if (path.find("../") != std::string::npos) return false;
  // more checks
  return true;
}

std::string http::GetMimeTypeByFileName(const std::string& file_name) {
  std::string extension = file_name.substr(file_name.find_last_of("."));
  if (extension == ".js") {
    return "application/javascript";
  } else if (extension == ".json") {
    return "application/json";
  } else if (extension == ".doc") {
    return "application/msword";
  } else if (extension == ".pdf") {
    return "application/pdf";
  } else if (extension == ".sql") {
    return "application/sql";
  } else if (extension == ".xls") {
    return "application/vnd.ms-excel";
  } else if (extension == ".ppt") {
    return "application/vnd.ms-powerpoint";
  } else if (extension == ".odt") {
    return "application/vnd.oasis.opendocument.text";
  } else if (extension == ".pptx") {
    return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  } else if (extension == ".xlsx") {
    return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  } else if (extension == ".docx") {
    return "application/vnd.openxmlformats-officedocument.wordprocessingml.documen";
  } else if (extension == ".xml") {
    return "application/xml";
  } else if (extension == ".zip") {
    return "application/zip";
  } else if (extension == ".zstd") {
    return "application/zstd";
  } else if (extension == ".bin") {
    return "application/macbinary";
  } else if (extension == ".mpeg") {
    return "audio/mpeg";
  } else if (extension == ".ogg") {
    return "audio/ogg";
  } else if (extension == ".apng") {
    return "image/apng";
  } else if (extension == ".avif") {
    return "image/avif";
  } else if (extension == ".flif") {
    return "image/flif";
  } else if (extension == ".gif") {
    return "image/gif";
  } else if (extension == ".jpg" || extension == ".jpeg" || extension == ".jiff" || extension == ".pjpeg" || extension == ".pjp") {
    return "image/jpeg";
  } else if (extension == ".jxl") {
    return "image/jxl";
  } else if (extension == ".png") {
    return "image/png";
  } else if (extension == ".svg") {
    return "image/svg+xml";
  } else if (extension == ".webp") {
    return "image/webp";
  } else if (extension == ".css") {
    return "text/css";
  } else if (extension == ".csv") {
    return "text/csv";
  } else if (extension == ".html" || extension == ".htm") {
    return "text/html";
  } else if (extension == ".php") {
    return "text/php";
  } else if (extension == ".txt") {
    return "text/plain";
  } else if (extension == ".xml") {
    return "text/xml";
  }
  return "*/*";
}

std::string http::GetReponseMessage(http::ResponseCode code) {
  return GetReponseMessage((int)code);
}

std::string http::GetReponseMessage(int code) {
  switch (code) {
    case CONTINUE:                        return "Continue";
    case SWITCHING_PROTOCOLS:             return "Switching Protocols";
    case PROCESSING:                      return "Processing";
    case EARLY_HINTS:                     return "Early Hints";
    case OK:                              return "OK";
    case CREATED:                         return "Created";
    case ACCEPTED:                        return "Accepted";
    case NON_AUTHORITATIVE_INFORMATION:   return "Non-Authoritative Information";
    case NO_CONTENT:                      return "No Content";
    case RESET_CONTENT:                   return "Reset Content";
    case PARTIAL_CONTENT:                 return "Partial Content";
    case MULTI_STATUS:                    return "Mutli-Status";
    case ALREADY_REPORTED:                return "Already Reported";
    case IM_USED:                         return "IM Used";
    case MULTIPLE_CHOICES:                return "Multiple Choices";
    case MOVED_PERMANENTLY:               return "Moved Permanently";
    case FOUND:                           return "Found";
    case NOT_MODIFIED:                    return "Not Modified";
    case USE_PROXY:                       return "Use Proxy";
    case SWITCH_PROXY:                    return "Switch Proxy";
    case TEMPORARY_REDIRECT:              return "Temporary Redirec";
    case PERMANENT_REDIRECT:              return "Permanent Redirect";
    case BAD_REQUEST:                     return "Bad Request";
    case UNAUTHORIZED:                    return "Unauthorized";
    case PAYMENT_REQUIRED:                return "Payment Required";
    case FORBIDDEN:                       return "Forbidden";
    case NOT_FOUND:                       return "Not Found";
    case METHOD_NOT_ALLOWED:              return "Method Not Allowed";
    case NOT_ACCEPTABLE:                  return "Not Acceptable";
    case PROXY_AUTHENTIFICATION_REQUIRED: return "Proxy Authentication Required";
    case REQUEST_TIMEOUT:                 return "Request Timeout";
    case CONFLICT:                        return "Conflict";
    case GONE:                            return "Gone";
    case LENGTH_REQUIRED:                 return "Length Required";
    case PRECONNECTION_FAILED:            return "Precondition Failed";
    case PAYLOAD_TOO_LARGE:               return "Payload Too Large";
    case URI_TOO_LONG:                    return "URI Too Long";
    case UNSUPPORTED_MEDIA_TYPE:          return "Unsupported Media Type";
    case RANGE_NOT_SATISFIABLE:           return "Range Not Satisfiable";
    case EXPECTATION_FAILED:              return "Expectation Failed";
    case IM_A_TEAPOT:                     return "I'm a teapot";
    case MISDIRECTED_REQUEST:             return "Misdirected Request";
    case UNPROCESSABLE_ENTITY:            return "Unprocessable Entity";
    case LOCKED:                          return "Locked";
    case FAILED_DEPENDENCY:               return "Failed Dependency";
    case TOO_EARLY:                       return "Too Early";
    case UPGRADE_REQUIRED:                return "Upgrade Required";
    case PRECONDITION_REQUIRED:           return "Precondition Required";
    case TOO_MANY_REQUESTS:               return "Too Many Requests";
    case REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
    case UNAVAILABLE_FOR_LEGAL_REASONS:   return "Unavailable For Legal Reasons";
    case INTERNAL_SERVER_ERROR:           return "Internal Server Error";
    case NOT_IMPLEMENTED:                 return "Not Implemented";
    case BAD_GATEWAY:                     return "Bad Gateway";
    case SERVICE_UNAVAILABLE:             return "Service Unavailable";
    case GATEWAY_TIMEOUT:                 return "Gateway Timeout";
    case HTTP_VERSION_NOT_SUPPORTED:      return "HTTP Version Not Supported";
    case VARIANT_ALSO_NEGOTIATES:         return "Variant Also Negotiates";
    case INSUFFICIENT_STORAGE:            return "Insufficient Storage";
    case LOOP_DETECTED:                   return "Loop Detected";
    case NOT_EXTENDED:                    return "Not Extended";
    case NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";
    default:                              return "?";
  }
}

std::string http::ResponseHeader::getString() const {
  std::stringstream ss;
  ss << "HTTP/" << http_version << " " << status << " " << GetReponseMessage(status);
  return ss.str();
}

http::Response::Response() {
  headers["Server"] = "mrthttp/1.0";
  headers["Connection"] = "close";
}

http::Response::Response(int status) : Response() {
  this->header.status = status;
}

http::Response& http::Response::setStatus(int status) {
  header.status = status;
  return *this;
}

http::Response& http::Response::addHeader(const std::string& key, const std::string& value) {
  headers[key] = value;
  return *this;
}

http::Response& http::Response::keepAlive(int max, int timeout) {
  headers["Connection"] = "keep-alive";
  headers["Keep-Alive"] = "max=" + std::to_string(max) + ", timeout=" + std::to_string(timeout);
  return *this;
}

http::Response& http::Response::setContent(const std::string& type, const std::string& content) {
  headers["Content-Type"] = type;
  headers["Conent-Length"] = std::to_string(content.size());
  body = content;
  return *this;
}

http::Response& http::Response::setContentFromFile(const std::string& file_path) {
  if (IsPathValid(file_path)) {
    std::ifstream content_file(file_path);
    if (!content_file.is_open()) {
      setStatus(NOT_FOUND).generateContent();
    } else {
      std::string content((std::istreambuf_iterator<char>(content_file)),
                           std::istreambuf_iterator<char>());
      setContent(GetMimeTypeByFileName(file_path), content);
    }
  } else {
    setStatus(NOT_FOUND).generateContent();
  }
  return *this;
}

http::Response& http::Response::generateContent() {
  setContent("text/plain", std::to_string(header.status) + " " + GetReponseMessage(header.status));
  return *this;
}

std::string http::Response::getString() const {
  std::stringstream ss;
  ss << header.getString() << kCRLF;
  for (auto& header_field : headers) {
    ss << header_field.first << ": " << header_field.second << kCRLF;
  }
  ss << kCRLF << body;
  return ss.str();
}

void http::Response::sendWithoutHandlerInvocation(net::Socket* socket) const {
  std::string response_str = getString();
#ifdef _DEBUG
  std::cout << "RESPONSE {\n" << response_str << "}" << std::endl;
#endif
  log::info("Sending %d to [%s]:%d", header.status, socket->getAddr().c_str(), socket->getPort());
  socket->send(response_str);
}

void http::Response::send(net::Socket* socket) {
  if (GetHandler(header.status) != nullptr) {
    *this = GetHandler(header.status)(*this);
  }
  sendWithoutHandlerInvocation(socket);
}
