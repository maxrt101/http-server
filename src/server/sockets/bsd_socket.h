#ifndef SERVER_SOCKETS_BSD_SOCKET_H_
#define SERVER_SOCKETS_BSD_SOCKET_H_

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server/sockets/socket.h"


namespace net {

class BsdSocket : public Socket {
 public:
  BsdSocket(sa_family_t family = AF_INET6);
  ~BsdSocket();

  BsdSocket& operator=(const BsdSocket& rhs);

  bool Create(int port) override;
  void Close() override;
  Socket* Accept() override;
  std::string Read(int size) override;
  void Send(const std::string& data) override;

  int GetFd() const override;
  int GetPort() const override;
  std::string GetAddr() const override;

 private:
  int fd_ = 0;
  sa_family_t family_;
  sockaddr_in address4_;
  sockaddr_in6 address6_;
};

} // namespace net

#endif

