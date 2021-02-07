/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Signal.h"
#include "WebSocketTypes.h"

struct Config;
class MessageInterface;

class WebSocketServer final {
 public:
  WebSocketServer(std::shared_ptr<asio::io_context> context, const Config& config);
  ~WebSocketServer();

  Signal<MessageInterface*> newConnection;

 private:
  WebSocketServerImpl mServer;
};
