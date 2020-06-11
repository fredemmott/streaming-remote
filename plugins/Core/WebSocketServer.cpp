/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "WebSocketServer.h"

#include <memory>

#include "Config.h"
#include "WebSocketConnection.h"

WebSocketServer::WebSocketServer(
  asio::io_context* context,
  const Config& config)
  : mServer() {
  mServer.clear_access_channels(websocketpp::log::alevel::all);
  mServer.clear_error_channels(websocketpp::log::elevel::all);
  mServer.init_asio(context);
  mServer.set_reuse_addr(true);
  mServer.set_open_handler([this](websocketpp::connection_hdl conn) {
    emit newConnection(new WebSocketConnection(&mServer, conn));
  });
  mServer.listen(config.webSocketPort);
  mServer.start_accept();
}

WebSocketServer::~WebSocketServer() {
}
