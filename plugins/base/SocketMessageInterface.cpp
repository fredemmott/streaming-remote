/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "SocketMessageInterface.h"

#include <QIODevice>

#define clean_and_return() \
  this->socket->close(); \
  this->deleteLater(); \
  return;

SocketMessageInterface::SocketMessageInterface(
  QIODevice* socket,
  QObject* parent)
  : socket(socket), MessageInterface(parent) {
  socket->setParent(this);
  connect(
    socket, &QIODevice::readyRead, this, &SocketMessageInterface::readyRead);
  this->socket = socket;
}

void SocketMessageInterface::readyRead() {
  auto s = this->socket;

  auto line = s->readLine();
  if (!(line.startsWith("Content-Length: ") && line.endsWith("\r\n"))) {
    clean_and_return();
  }
  auto next = s->readLine();
  if (next != "\r\n") {
    clean_and_return();
  }

  auto message_len
    = line.mid(16 /* "Content-Length: */, line.length() - 18).toInt();
  QByteArray message;
  while (message_len > 0) {
    auto chunk = s->read(message_len);
    message += chunk;
    message_len -= chunk.length();
  }

  emit messageReceived(message);
}

void SocketMessageInterface::sendMessage(const QByteArray& message) {
  this->socket->write(
    QString("Content-Length: %1\r\n\r\n").arg(message.length()).toUtf8()
    + message);
}
