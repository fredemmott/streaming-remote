/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "SocketServer.h"

#include "ClientHandler.h"
#include "SocketMessageInterface.h"
#include "StreamingSoftware.h"
#include "WebSocketMessageInterface.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWebSocket>
#include <QWebSocketServer>

#include <sodium.h>

#include <QLoggingCategory>

SocketServer::SocketServer(StreamingSoftware* parent):
  QObject(parent),
  software(parent),
  localServer(nullptr),
  tcpServer(nullptr),
  webSocketServer(nullptr) {

  const auto result = sodium_init();
  assert(result == 0);

  connect(software, &StreamingSoftware::configurationChanged, this, &SocketServer::startListening);
}

void SocketServer::startListening() {
  this->stopListening();

  const auto config = this->software->getConfiguration();
  if (!config.localSocket.isNull()) {
    if (localServer) {
      if (localServer->serverName() != config.localSocket) {
        localServer->close();
        localServer->listen(config.localSocket);
      }
    } else {
      localServer = new QLocalServer(this);
      connect(localServer, &QLocalServer::newConnection, this, &SocketServer::newLocalConnection);
      #ifndef WIN32
        QFile::remove(config.localSocket);
      #endif
      localServer->listen(config.localSocket);
    }
  } else if (localServer) {
    localServer->close();
    localServer->deleteLater();
    localServer = nullptr;
  }

  if (config.tcpPort) {
    if (tcpServer) {
      if (tcpServer->serverPort() != config.tcpPort) {
        tcpServer->close();
        tcpServer->listen(QHostAddress::Any, config.tcpPort);
      }
    } else {
      tcpServer = new QTcpServer(this);
      connect(this->tcpServer, &QTcpServer::newConnection, this, &SocketServer::newTcpConnection);
      tcpServer->listen(QHostAddress::Any, config.tcpPort);
    }
  } else if (tcpServer) {
    tcpServer->close();
    tcpServer->deleteLater();
    tcpServer = nullptr;
  }

  if (config.webSocketPort) {
    if (webSocketServer) {
      if (webSocketServer->serverPort() != config.webSocketPort) {
        webSocketServer->close();
        webSocketServer->listen(QHostAddress::Any, config.webSocketPort);
      }
    } else {
      webSocketServer = new QWebSocketServer("", QWebSocketServer::NonSecureMode, this);
      connect(this->webSocketServer, &QWebSocketServer::newConnection, this, &SocketServer::newWebSocketConnection);
      webSocketServer->listen(QHostAddress::Any, config.webSocketPort);
    }
  } else if (webSocketServer) {
    webSocketServer->close();
    webSocketServer->deleteLater();
    webSocketServer = nullptr;
  }
}

void SocketServer::stopListening() {
  delete localServer;
  delete tcpServer;
  delete webSocketServer;
  localServer = nullptr;
  tcpServer = nullptr;
  webSocketServer = nullptr;
}

void SocketServer::newLocalConnection() {
  auto socket = this->localServer->nextPendingConnection();
  auto mi = new SocketMessageInterface(socket, this);
  auto handler = new ClientHandler(this->software);
  mi->setParent(handler);
  connect(mi, &MessageInterface::messageReceived, handler, &ClientHandler::messageReceived);
  connect(handler, &ClientHandler::sendMessage, mi, &MessageInterface::sendMessage);
}

void SocketServer::newTcpConnection() {
	auto socket = this->tcpServer->nextPendingConnection();
	auto mi = new SocketMessageInterface(socket, this);
	auto handler = new ClientHandler(this->software);
  mi->setParent(handler);
	connect(mi, &MessageInterface::messageReceived, handler, &ClientHandler::messageReceived);
	connect(handler, &ClientHandler::sendMessage, mi, &MessageInterface::sendMessage);
}

void SocketServer::newWebSocketConnection() {
  auto socket = this->webSocketServer->nextPendingConnection();
  auto mi = new WebSocketMessageInterface(socket, this);
  auto handler = new ClientHandler(this->software);
  mi->setParent(handler);
  connect(mi, &MessageInterface::messageReceived, handler, &ClientHandler::messageReceived);
  connect(handler, &ClientHandler::sendMessage, mi, &MessageInterface::sendMessage);
}
