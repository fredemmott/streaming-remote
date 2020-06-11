/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "TCPServer.h"

#include <memory>

#include "Config.h"
#include "TCPConnection.h"

TCPServer::TCPServer(asio::io_context* context, const Config& config)
  : mContext(context),
    mAcceptor(asio::ip::tcp::acceptor(
      *context,
      asio::ip::tcp::endpoint(asio::ip::tcp::v6(), config.tcpPort),
      true)) {
  startAccept();
}

TCPServer::~TCPServer() {
}

void TCPServer::startAccept() {
  auto conn = new TCPConnection(mContext);
  mAcceptor.async_accept(conn->socket(), [=](const asio::error_code& error) {
    this->newConnection(conn);
    this->startAccept();
  });
}
