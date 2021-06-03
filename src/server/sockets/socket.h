#ifndef SERVER_SOCKETS_SOCKET_H_
#define SERVER_SOCKETS_SOCKET_H_

#include <string>

namespace net {

class Socket {
 public:
  virtual ~Socket();

  virtual bool Create(int port) = 0;
  virtual void Close() = 0;
  // virtual void Connect() = 0; // for future
  virtual Socket* Accept() = 0;
  virtual std::string Read(int size) = 0;
  virtual void Send(const std::string& data) = 0;

  virtual int GetFd() const = 0;
  virtual int GetPort() const = 0;
  virtual std::string GetAddr() const = 0;
};

} // namespace net

#endif

