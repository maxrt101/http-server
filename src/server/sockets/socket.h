#ifndef SERVER_SOCKETS_SOCKET_H_
#define SERVER_SOCKETS_SOCKET_H_

#include <string>

namespace net {

class Socket {
 public:
  virtual ~Socket();

  virtual bool create(int port) = 0;
  virtual void close() = 0;
  // virtual void Connect() = 0; // for future
  virtual Socket* accept() = 0;
  virtual std::string read(int size) = 0;
  virtual void send(const std::string& data) = 0;

  virtual int getFd() const = 0;
  virtual int getPort() const = 0;
  virtual std::string getAddr() const = 0;
};

} // namespace net

#endif

