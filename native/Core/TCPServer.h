/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Signal.h"

#include <asio.hpp>

struct Config;
class MessageInterface;

class TCPServer final {
 public:
  TCPServer(std::shared_ptr<asio::io_context> context, const Config& config);
  ~TCPServer();

  Signal<MessageInterface*> newConnection;

 private:
  void startAccept();
  asio::ip::tcp::acceptor mAcceptor;
  std::shared_ptr<asio::io_context> mContext;
};
