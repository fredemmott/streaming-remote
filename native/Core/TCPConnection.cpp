/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "TCPConnection.h"

#include <fmt/format.h>
#include <asio.hpp>
#include <istream>

TCPConnection::TCPConnection(std::shared_ptr<asio::io_context> ctx)
  : MessageInterface(), mSocket(*ctx) {
}

void TCPConnection::startWaitingForMessage() {
  mSocket.async_wait(
    asio::socket_base::wait_type::wait_read,
    std::bind(&TCPConnection::readyRead, this)
  );
  mSocket.async_wait(
    asio::socket_base::wait_type::wait_error,
    [this](asio::error_code) { emit this->disconnected(); }
  );
}

void TCPConnection::readyRead() {
  asio::streambuf buf;
  std::istream bufreader(&buf);
  auto& s = this->mSocket;

  asio::read_until(s, buf, "\r\n");
  std::string line;
  bufreader >> line;
  if (!line.starts_with("Content-Length: ")) {
    delete this;
    return;
  }
  asio::read_until(s, buf, "\r\n");
  std::string next;
  bufreader >> next;
  if (next != "\r\n") {
    delete this;
    return;
  }

  auto message_len
    = std::stoi(line.substr(16 /* "Content-Length: */, line.length() - 18));
  asio::read(s, buf, asio::transfer_exactly(message_len));
  std::string message;
  bufreader >> message;
  emit messageReceived(message);
  startWaitingForMessage();
}

void TCPConnection::sendMessage(const std::string& message) {
  std::string buf
    = fmt::format("Content-Length: {}\r\n\r\n{}", message.size(), message);
  asio::write(mSocket, asio::buffer(buf), asio::transfer_exactly(buf.size()));
}

asio::ip::tcp::socket& TCPConnection::socket() {
  return mSocket;
}
