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

class Server {
 public:
  Server(asio::io_context* context, StreamingSoftware* software);
  virtual ~Server();

  virtual void startListening(const Config& config);
  virtual void stopListening();

 protected:
  void newConnection(MessageInterface* connection);

 private:
  asio::io_context* mContext;
  StreamingSoftware* mSoftware;
  TCPServer* mTCPServer = nullptr;
  WebSocketServer* mWebSocketServer = nullptr;
};
