#include "mrt/server/server/http_server.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>

#include "mrt/server/sockets/bsd_socket.h"
#include "mrt/server/sockets/socket.h"
#include "mrt/server/http/response.h"
#include "mrt/server/http/request.h"
#include "mrt/server/http/parser.h"
#include "mrt/server/debug/log.h"
#include "mrt/server/utils/die.h"

std::atomic<bool> terminate_flag = false;


net::HttpServer::HttpServer() {}

net::HttpServer::HttpServer(config::HttpServerCofig conf) : m_conf(conf) {}

net::HttpServer::~HttpServer() {}

void net::HttpServer::init() {
  if (!m_socket.create(m_conf.port)) {
    utils::Die();
  }
}

void net::HttpServer::init(config::HttpServerCofig conf) {
  m_conf = conf;
  init();
}

void net::HttpServer::run() {
  log::info("Listening for incoming connections");

  while (1) {
    net::Socket* client = m_socket.accept();
    TaskParams* params = new TaskParams{this, client};
    m_pool.addTask({&HttpServer::dealWithClientTask, params});
  }
}

void net::HttpServer::stop() {
  terminate_flag.store(true);
  m_pool.waitForAll();
}

void net::HttpServer::addEndpoint(Endpoint endpoint) {
  m_endpoints[endpoint.url][(int)endpoint.method] = endpoint;
}

void net::HttpServer::dealWithClientTask(void* arg) {
  TaskParams* params = static_cast<TaskParams*>(arg);
  params->server->dealWithClient(params->client);
  delete params;
}

void net::HttpServer::dealWithClient(net::Socket* client) {
  if (client->getFd() < 0) {
    log::error("accept() failed: %s", strerror(errno));
    delete client;
    return;
  }

  log::info("Accepted connection from [%s]:%d", client->getAddr().c_str(), client->getPort());

  bool keep_alive = false;
  int keep_alive_transaction_count = 0;

  do {
    http::RequestParser parser;
    auto result = parser.parse(client);

    if (!result) {
      if (result.error == http::RequestParser::Error::kInterrupted) {
        log::warning("Request parsing was interrupted ([%s]:%d)", client->getAddr().c_str(), client->getPort());
      } else if (result.error == http::RequestParser::Error::kNoData) {
        log::warning("Request from [%s]:%d is not complete", client->getAddr().c_str(), client->getPort());
      } else if (result.error == http::RequestParser::Error::kRequestTimeout) {
        log::warning("Request from [%s]:%d timed out", client->getAddr().c_str(), client->getPort());
        http::Response(http::REQUEST_TIMEOUT).generateContent().send(client);
      } else if (result.error == http::RequestParser::Error::kBadRequest) {
        log::warning("Request parsing failed - invalid request ([%s]:%d)", client->getAddr().c_str(), client->getPort());
        http::Response(http::BAD_REQUEST).generateContent().send(client);
      } else {
        log::warning("Error parsing (%d)", result.error);
      }
      break;
    }

    if (result.request.headers.find("Connection") != result.request.headers.end()) {
      if (result.request.headers["Connection"] == "keep-alive") {
        // keep_alive = true;
      }
    }

#ifdef _DEBUG
    std::cout << "REQUEST: {\n" << result.request.GetString() << "}" << std::endl;
#endif

    auto itr = m_endpoints.find(result.request.header.url);
    if (itr != m_endpoints.end()) {
      Endpoint endpoint;
      auto& endpoints_url = m_endpoints[result.request.header.url];
      if (endpoints_url.find((int)result.request.header.method) != endpoints_url.end()) {
        endpoint = endpoints_url[(int)result.request.header.method];
      } else if (endpoints_url.find((int)http::Method::NONE) != endpoints_url.end()) {
        endpoint = endpoints_url[(int)http::Method::NONE];
      } else {
        http::Response(http::METHOD_NOT_ALLOWED).generateContent().send(client);
      }

      log::debug("Endpoint: {%s, %s, %i}", endpoint.url.c_str(), http::GetMethodName(endpoint.method).c_str(), endpoint.function != nullptr);

      if (endpoint.function != nullptr) {
        http::Response response = endpoint.function(result.request);
        log::debug("Keep-Alive: $d", keep_alive);
        if (keep_alive) {
          response.keepAlive(m_conf.keep_alive_max, m_conf.keep_alive_timeout);
        }
        response.send(client);
      } else {
        http::Response(http::NOT_FOUND).generateContent().send(client);
      }
    } else {
      if (!m_conf.root_folder.empty()) {
        if (result.request.header.method == http::Method::GET) {
          std::string url = result.request.header.url;
          if (url == "/") {
            url += m_conf.default_doc;
          }
          http::Response response;
          response.setContentFromFile(url);
          if (keep_alive) {
            response.keepAlive(m_conf.keep_alive_max, m_conf.keep_alive_timeout);
          }
          response.send(client);
        }
      } else {
        http::Response(http::NOT_FOUND).generateContent().send(client);
      }
    }

    /* Keep-Alive */
    keep_alive_transaction_count++;

    if (!keep_alive || keep_alive_transaction_count >= m_conf.keep_alive_max) {
      break;
    }

    // for (int i = 0; i < conf_.keep_alive_timeout; i++) {
      pollfd fds;
      fds.fd = client->getFd();
      fds.events = POLLIN;

      int poll_ret = poll(&fds, 1, m_conf.keep_alive_timeout * 1000);
      if (terminate_flag.load()) {
        keep_alive = false;
        break;
      }

      if (poll_ret == -1) {
        log::warning("poll() failed during keep-alive wait, connection to [%s]:%d closing: %s",
          client->getAddr().c_str(), client->getPort(), strerror(errno));
        keep_alive = false;
        break;
      } else if (poll_ret == 0) {
        log::debug("Keep-Alive Timeout");
      } else if (fds.revents & POLLIN) {
        log::debug("Another request from [%s]:%d", client->getAddr().c_str(), client->getPort());
        break;
      }
    // }

  } while (keep_alive);

  client->close();
  delete client;
}
