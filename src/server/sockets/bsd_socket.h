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

  bool create(int port) override;
  void close() override;
  Socket* accept() override;
  std::string read(int size) override;
  void send(const std::string& data) override;

  int getFd() const override;
  int getPort() const override;
  std::string getAddr() const override;

 private:
  int m_fd = 0;
  sa_family_t m_family;
  sockaddr_in m_address4;
  sockaddr_in6 m_address6;
};

} // namespace net

#endif

