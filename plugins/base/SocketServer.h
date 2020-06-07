/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "ClientHandler.h"

#include <QObject>
#include <QPointer>
#include <QVector>

class QLocalServer;
class QTcpServer;
class QWebSocketServer;

class MessageInterface;
class SocketHandler;
class StreamingSoftware;

class SocketServer : public QObject {
  Q_OBJECT

 public:
  explicit SocketServer(StreamingSoftware* parent);
 public slots:
  void startListening();
  void stopListening();
 private slots:
  void newLocalConnection();
  void newTcpConnection();
  void newWebSocketConnection();

 private:
  QLocalServer* localServer = nullptr;
  QTcpServer* tcpServer = nullptr;
  QWebSocketServer* webSocketServer = nullptr;

  StreamingSoftware* software;
};
