#include "mrt/server/sockets/bsd_socket.h"

#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "mrt/server/sockets/socket.h"
#include "mrt/server/debug/log.h"
#include "mrt/server/utils/die.h"

static constexpr int kIPv6AddressBufferSize = INET6_ADDRSTRLEN+1;

net::BsdSocket::BsdSocket(sa_family_t family) : m_family(family) {}

net::BsdSocket::~BsdSocket() {}


net::BsdSocket& net::BsdSocket::operator=(const BsdSocket& rhs) {
  m_fd = rhs.m_fd;
  m_address4 = rhs.m_address4;
  m_address6 = rhs.m_address6;
  return *this;
}


bool net::BsdSocket::create(int port) {
  m_fd = socket(m_family, SOCK_STREAM, 0);

  if (m_fd < 0) {
    log::error("Could not create socket: %s", strerror(errno));
    if (m_family == AF_INET6 && errno == EAFNOSUPPORT) {
      log::info("Trying to create IPv4 socket instead");
      m_family = AF_INET;
      m_fd = socket(m_family, SOCK_STREAM, 0);
      if (m_fd < 0) {
        log::error("Could not create socket: %s", strerror(errno));
        return false;
      }
    } else {
      return false;
    }
  }

  if (m_family == AF_INET) {
    m_address4.sin_family = m_family;
    m_address4.sin_port = htons(port);
    m_address4.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(m_fd, (sockaddr*)&m_address4, sizeof(sockaddr_in) < 0)) {
      log::error("bind() failed: %s", strerror(errno));
      return false;
    }
    log::info("Socket is IPv4");
  } else if (m_family == AF_INET6) {
    m_address6.sin6_family = m_family;
    m_address6.sin6_port = htons(port);
    m_address6.sin6_addr = in6addr_any;
    if (::bind(m_fd, (sockaddr*)&m_address6, sizeof(sockaddr_in6)) < 0) {
      log::error("bind() failed: %s",  strerror(errno));
      return false;
    }
    log::info("Socket is IPv6");
  } else {
    log::info("Invalid addresss family");
    return false;
  }

  if (listen(m_fd, SOMAXCONN) < 0) {
    log::error("listen() failed: %s", strerror(errno));
    return false;
  }
  
  return true;
}


void net::BsdSocket::close() {
  ::close(m_fd);
}


net::Socket* net::BsdSocket::accept() {
  BsdSocket* client = new BsdSocket(m_family);

  if (m_family == AF_INET) {
    socklen_t addrlen = sizeof(sockaddr_in);
    client->m_fd = ::accept(m_fd, (sockaddr*)&client->m_address4, &addrlen);
  } else if (m_family == AF_INET6) {
    socklen_t addrlen = sizeof(sockaddr_in6);
    client->m_fd = ::accept(m_fd, (sockaddr*)&client->m_address6, &addrlen);
  } else {
    log::error("Invalid address family");
    delete client;
    return nullptr;
  }

  return client;
}


std::string net::BsdSocket::read(int size) {
  char* buffer = new char[size];
  std::string data;
  int read_len = ::read(m_fd, buffer, size);
  data = buffer;
  delete [] buffer;
  return data;
}


void net::BsdSocket::send(const std::string& data) {
  write(m_fd, data.c_str(), data.size());
}


int net::BsdSocket::getFd() const {
  return m_fd;
}


int net::BsdSocket::getPort() const {
  if (m_family == AF_INET) {
    return ntohs(m_address4.sin_port);
  } else if (m_family == AF_INET6) {
    return ntohs(m_address6.sin6_port);
  }
  return 0;
}


std::string net::BsdSocket::getAddr() const {
  char addrstrbuf[kIPv6AddressBufferSize] {0};
  const char* ntop_ret = 0;
  if (m_family == AF_INET) {
    ntop_ret = inet_ntop(m_family, &m_address4.sin_addr, addrstrbuf, kIPv6AddressBufferSize);
  } else if (m_family == AF_INET6) {
    ntop_ret = inet_ntop(m_family, &m_address6.sin6_addr, addrstrbuf, kIPv6AddressBufferSize);
  }

  if (ntop_ret == addrstrbuf) {
    return std::string(addrstrbuf);
  } else {
    log::error("inet_notp() failed: %s", strerror(errno));
  }
  return "";
}


