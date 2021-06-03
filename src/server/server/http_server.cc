#include "server/server/http_server.h"

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

#include "server/sockets/bsd_socket.h"
#include "server/sockets/socket.h"
#include "server/http/response.h"
#include "server/http/request.h"
#include "server/http/parser.h"
#include "server/debug/log.h"
#include "server/utils/die.h"

// std::mutex data_access_mutex;
// std::mutex socket_access_mutex;
std::atomic<bool> terminate_flag = false;

net::HttpServer::HttpServer() {}

net::HttpServer::HttpServer(config::HttpServerCofig conf) : conf_(conf) {}

net::HttpServer::~HttpServer() {}

void net::HttpServer::Init() {
  if (!socket_.Create(conf_.port)) {
    utils::Die();
  }
}

void net::HttpServer::Init(config::HttpServerCofig conf) {
  conf_ = conf;
  Init();
}

void net::HttpServer::Run() {
  log::Info("Listening for incoming connections");

  while (1) {
    net::Socket* client = socket_.Accept();
    JobParams* params = new JobParams{this, client};
    pool_.AddJob(mrt::threads::Job(&HttpServer::DealWithClientJob, params));
  }
}

void net::HttpServer::Stop() {
  terminate_flag.store(true);
  pool_.WaitForAll();
}

void net::HttpServer::AddEndpoint(Endpoint endpoint) {
  endpoints_[endpoint.url][(int)endpoint.method] = endpoint;
}

void net::HttpServer::DealWithClientJob(void* arg) {
  JobParams* params = static_cast<JobParams*>(arg);
  params->server->DealWithClient(params->client);
  delete params;
}

void net::HttpServer::DealWithClient(net::Socket* client) {
  if (client->GetFd() < 0) {
    log::Error("accept() failed: %s", strerror(errno));
    delete client;
    return;
  }

  log::Info("Accepted connection from [%s]:%d", client->GetAddr().c_str(), client->GetPort());

  bool keep_alive = false;
  int keep_alive_transaction_count = 0;

  do {
    http::RequestParser parser;
    auto result = parser.Parse(client);

    if (!result) {
      if (result.error == http::RequestParser::Error::kInterrupted) {
        log::Warning("Request parsing was interrupted ([%s]:%d)", client->GetAddr().c_str(), client->GetPort());
      } else if (result.error == http::RequestParser::Error::kNoData) {
        log::Warning("Request from [%s]:%d is not complete", client->GetAddr().c_str(), client->GetPort());
      } else if (result.error == http::RequestParser::Error::kRequestTimeout) {
        log::Warning("Request from [%s]:%d timed out", client->GetAddr().c_str(), client->GetPort());
        http::Response(http::REQUEST_TIMEOUT).GenerateContent().Send(client);
      } else if (result.error == http::RequestParser::Error::kBadRequest) {
        log::Warning("Request parsing failed - invalid request ([%s]:%d)", client->GetAddr().c_str(), client->GetPort());
        http::Response(http::BAD_REQUEST).GenerateContent().Send(client);
      } else {
        log::Warning("Error parsing (%d)", result.error);
      }
      break;
    }

    if (result.request.headers.find("Connection") != result.request.headers.end()) {
      if (result.request.headers["Connection"] == "keep-alive") {
        // keep_alive = true;
      }
    }

    std::cout << "REQUEST: {\n" << result.request.GetString() << "}" << std::endl;

    auto itr = endpoints_.find(result.request.header.url);
    if (itr != endpoints_.end()) {
      Endpoint endpoint;
      auto& endpoints_url = endpoints_[result.request.header.url];
      if (endpoints_url.find((int)result.request.header.method) != endpoints_url.end()) {
        endpoint = endpoints_url[(int)result.request.header.method];
      } else if (endpoints_url.find((int)http::Method::NONE) != endpoints_url.end()) {
        endpoint = endpoints_url[(int)http::Method::NONE];
      } else {
        http::Response(http::METHOD_NOT_ALLOWED).GenerateContent().Send(client);
      }

      log::Debug("Endpoint: {%s, %s, %i}", endpoint.url.c_str(), http::GetMethodName(endpoint.method).c_str(), endpoint.function != nullptr);

      if (endpoint.function != nullptr) { 
        http::Response response = endpoint.function(result.request);
        if (keep_alive) {
          response.KeepAlive(conf_.keep_alive_max, conf_.keep_alive_timeout);
        }
        response.Send(client);
      } else {
        http::Response(http::NOT_FOUND).GenerateContent().Send(client);
      }
    } else {
      if (!conf_.root_folder.empty()) {
        if (result.request.header.method == http::Method::GET) {
          std::string url = result.request.header.url;
          if (url == "/") {
            url += conf_.default_doc;
          }
          http::Response response;
          response.SetContentFromFile(url);
          if (keep_alive) {
            response.KeepAlive(conf_.keep_alive_max, conf_.keep_alive_timeout);
          }
          response.Send(client);
        }
      } else {
        http::Response(http::NOT_FOUND).GenerateContent().Send(client);
      }
    }

    /* Keep-Alive */
    keep_alive_transaction_count++;

    if (!keep_alive || keep_alive_transaction_count >= conf_.keep_alive_max) {
      break;
    }

    for (int i = 0; i < conf_.keep_alive_timeout; i++) {
      pollfd fds;
      fds.fd = client->GetFd();
      fds.events = POLLIN;

      int poll_ret = poll(&fds, 1, 1000);
      if (terminate_flag.load()) {
        keep_alive = false;
        break;
      }

      if (poll_ret == -1) {
        log::Warning("poll() failed during keep-alive wait, connection to [%s]:%d closing: %s",
          client->GetAddr().c_str(), client->GetPort(), strerror(errno));
        keep_alive = false;
        break;
      } else if (fds.revents & POLLIN) {
        log::Debug("Another request from [%s]:%d", client->GetAddr().c_str(), client->GetPort());
        break;
      }
    }

  } while (keep_alive);

  client->Close();
  delete client;
}