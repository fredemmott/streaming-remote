/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "MessageInterface.h"
#include "WebSocketTypes.h"

class WebSocketConnection : public MessageInterface {
 public:
  WebSocketConnection(
    WebSocketServerImpl* server,
    websocketpp::connection_hdl connection);
  ~WebSocketConnection();

  void sendMessage(const std::string& message);

 private:
  WebSocketServerImpl* mServer;
  websocketpp::connection_hdl mConnection;
};
