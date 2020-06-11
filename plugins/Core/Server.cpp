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
    mTCPServer = new TCPServer(mContext, config);
    mTCPServer->newConnection.connect(this, &Server::newConnection);
  }
  if (config.webSocketPort) {
    mWebSocketServer = new WebSocketServer(mContext, config);
    mWebSocketServer->newConnection.connect(this, &Server::newConnection);
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
