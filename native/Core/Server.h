/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

struct Config;
class MessageInterface;
class StreamingSoftware;
class TCPServer;
class WebSocketServer;

namespace asio {
class io_context;
}

#include <memory>

class Server {
 public:
  Server(
    std::shared_ptr<asio::io_context> context,
    std::shared_ptr<StreamingSoftware> software
  );
  virtual ~Server();

  virtual void startListening(const Config& config);
  virtual void stopListening();

 protected:
  void newConnection(MessageInterface* connection);

 private:
  std::shared_ptr<asio::io_context> mContext;
  std::shared_ptr<StreamingSoftware> mSoftware;
  TCPServer* mTCPServer = nullptr;
  WebSocketServer* mWebSocketServer = nullptr;
};
