/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "WebSocketMessageInterface.h"

#include <QWebSocket>

#define clean_and_return() \
  this->socket->close(); \
  this->deleteLater(); \
  return;

WebSocketMessageInterface::WebSocketMessageInterface(
  QWebSocket* socket,
  QObject* parent)
  : socket(socket), MessageInterface(parent) {
  socket->setParent(this);
  connect(
    socket, &QWebSocket::binaryMessageReceived, this,
    &MessageInterface::messageReceived);
  this->socket = socket;
}

void WebSocketMessageInterface::sendMessage(const QByteArray& message) {
  this->socket->sendBinaryMessage(message);
}
