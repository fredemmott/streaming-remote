/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include <sodium.h>

#include "ClientHandler.h"
#include "Config.h"
#include "Logger.h"
#include "MessageInterface.h"
#include "StreamingSoftware.h"
#include "TCPServer.h"
#include "WebSocketServer.h"

#include "Server.h"

Server::Server(asio::io_context* context, StreamingSoftware* software)
  : mContext(context), mSoftware(software) {
  const auto result = sodium_init();
  assert(result == 0 /* init */ || result == 1 /* already done */);
  software->configurationChanged.connect(this, &Server::startListening);
}

Server::~Server() {
}

void Server::startListening(const Config& config) {
  stopListening();
  if (config.tcpPort) {
    try {
      mTCPServer = new TCPServer(mContext, config);
      mTCPServer->newConnection.connect(this, &Server::newConnection);
    } catch (const std::system_error& e) {
      if (e.code() == std::errc::address_in_use) {
        Logger::debug(
          "Failed to start TCP server: port {} is already in use.",
          config.tcpPort
        );
      } else {
        Logger::debug("Failed to start TCP server: {}", e.what());
      }
    }
  }
  if (config.webSocketPort) {
    try {
      mWebSocketServer = new WebSocketServer(mContext, config);
      mWebSocketServer->newConnection.connect(this, &Server::newConnection);
    } catch (const websocketpp::exception& e) {
      if (e.code() == asio::error::address_in_use) {
        Logger::debug(
          "Failed to start WebSocket server: port {} is already in use.",
          config.webSocketPort
        );
      } else {
        Logger::debug("Failed to start WebSocket server: {}", e.what());
      }
    }
  }
}

void Server::stopListening() {
  delete mTCPServer;
  mTCPServer = nullptr;
  delete mWebSocketServer;
  mWebSocketServer = nullptr;
}

void Server::newConnection(MessageInterface* connection) {
  new ClientHandler(mSoftware, connection);
}
