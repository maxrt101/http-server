#include "server/sockets/bsd_socket.h"

#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "server/sockets/socket.h"
#include "server/debug/log.h"
#include "server/utils/die.h"


static constexpr int kIPv6AddressBufferSize = INET6_ADDRSTRLEN+1;


net::BsdSocket::BsdSocket(sa_family_t family) : family_(family) {}

net::BsdSocket::~BsdSocket() {}


net::BsdSocket& net::BsdSocket::operator=(const BsdSocket& rhs) {
  fd_ = rhs.fd_;
  address4_ = rhs.address4_;
  address6_ = rhs.address6_;
  return *this;
}


bool net::BsdSocket::Create(int port) {
  fd_ = socket(family_, SOCK_STREAM, 0);

  if (fd_ < 0) {
    log::Error("Could not create socket: %s", strerror(errno));
    if (family_ == AF_INET6 && errno == EAFNOSUPPORT) {
      log::Info("Trying to create IPv4 socket instead");
      family_ = AF_INET;
      fd_ = socket(family_, SOCK_STREAM, 0);
      if (fd_ < 0) {
        log::Error("Could not create socket: %s", strerror(errno));
        return false;
      }
    } else {
      return false;
    }
  }

  if (family_ == AF_INET) {
    address4_.sin_family = family_;
    address4_.sin_port = htons(port);
    address4_.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd_, (sockaddr*)&address4_, sizeof(sockaddr_in) < 0)) {
      log::Error("bind() failed: %s", strerror(errno));
      return false;
    }
    log::Info("Socket is IPv4");
  } else if (family_ == AF_INET6) {
    address6_.sin6_family = family_;
    address6_.sin6_port = htons(port);
    address6_.sin6_addr = in6addr_any;
    if (bind(fd_, (sockaddr*)&address6_, sizeof(sockaddr_in6)) < 0) {
      log::Error("bind() failed: %s",  strerror(errno));
      return false;
    }
    log::Info("Socket is IPv6");
  } else {
    log::Info("Invalid addresss family");
    return false;
  }

  if (listen(fd_, SOMAXCONN) < 0) {
    log::Error("listen() failed: %s", strerror(errno));
    return false;
  }
  
  return true;
}


void net::BsdSocket::Close() {
  close(fd_);
}


net::Socket* net::BsdSocket::Accept() {
  BsdSocket* client = new BsdSocket(family_);

  if (family_ == AF_INET) {
    socklen_t addrlen = sizeof(sockaddr_in);
    client->fd_ = accept(fd_, (sockaddr*)&client->address4_, &addrlen);
  } else if (family_ == AF_INET6) {
    socklen_t addrlen = sizeof(sockaddr_in6);
    client->fd_ = accept(fd_, (sockaddr*)&client->address6_, &addrlen);
  } else {
    log::Error("Invalid address family");
    delete client;
    return nullptr;
  }

  return client;
}


std::string net::BsdSocket::Read(int size) {
  char* buffer = new char[size];
  std::string data;
  int read_len = read(fd_, buffer, size);
  data = buffer;
  delete [] buffer;
  return data;
}


void net::BsdSocket::Send(const std::string& data) {
  write(fd_, data.c_str(), data.size());
}


int net::BsdSocket::GetFd() const {
  return fd_;
}


int net::BsdSocket::GetPort() const {
  if (family_ == AF_INET) {
    return ntohs(address4_.sin_port);
  } else if (family_ == AF_INET6) {
    return ntohs(address6_.sin6_port);
  }
  return 0;
}


std::string net::BsdSocket::GetAddr() const {
  char addrstrbuf[kIPv6AddressBufferSize] {0};
  const char* ntop_ret = 0;
  if (family_ == AF_INET) {
    ntop_ret = inet_ntop(family_, &address4_.sin_addr, addrstrbuf, kIPv6AddressBufferSize);
  } else if (family_ == AF_INET6) {
    ntop_ret = inet_ntop(family_, &address6_.sin6_addr, addrstrbuf, kIPv6AddressBufferSize);
  }

  if (ntop_ret == addrstrbuf) {
    return std::string(addrstrbuf);
  } else {
    log::Error("inet_notp() failed: %s", strerror(errno));
  }
  return "";
}


