/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "MessageInterface.h"

class QWebSocket;

class WebSocketMessageInterface : public MessageInterface{
  Q_OBJECT

  public:
    explicit WebSocketMessageInterface(QWebSocket* socket, QObject* parent = nullptr);
  public slots:
    void sendMessage(const QByteArray& message);
  private:
    QWebSocket* socket;
};
