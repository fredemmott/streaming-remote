/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "WebSocketConnection.h"

#include <fmt/format.h>
#include <asio.hpp>
#include <istream>

WebSocketConnection::WebSocketConnection(
  WebSocketServerImpl* server,
  websocketpp::connection_hdl hdl)
  : MessageInterface(), mServer(server), mConnection(hdl) {
  auto conn = server->get_con_from_hdl(hdl);
  conn->set_message_handler(
    [this](
      websocketpp::connection_hdl, WebSocketServerImpl::message_ptr message) {
      if (!message) {
        return;
      }
      if (message->get_opcode() != websocketpp::frame::opcode::binary) {
        return;
      }
      const auto data = message->get_payload();
      emit messageReceived(data);
    });
  conn->set_close_handler([this](websocketpp::connection_hdl) { emit disconnected(); });
}

WebSocketConnection::~WebSocketConnection() {
}

void WebSocketConnection::sendMessage(const std::string& message) {
  websocketpp::lib::error_code error;
  mServer->send(
    mConnection, message, websocketpp::frame::opcode::binary, error);
  if (error) {
    delete this;
  }
}
