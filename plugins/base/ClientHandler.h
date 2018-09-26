/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "StreamingSoftware.h"

#include "ClientState.h"

#include <QObject>
#include <QPointer>

#include <sodium.h>

class QJsonValue;

class ClientHandler : public QObject {
  Q_OBJECT
  public:
    explicit ClientHandler(StreamingSoftware* parent);
    ~ClientHandler();

  public slots:
    void messageReceived(const QByteArray& message);
  signals:
    void sendMessage(const QByteArray& message);
  private slots:
    void outputStateChanged(const QString& id, OutputState state);
  private:
    void handshakeClientHelloMessageReceived(const QByteArray& message);
    void handshakeClientReadyMessageReceived(const QByteArray& message);
    void encryptedRpcMessageReceived(const QByteArray& message);
    void plaintextRpcMessageReceived(const QByteArray& message);
    void encryptThenSendMessage(const QByteArray& message);
    void cleanCrypto();
    void cleanCryptoKeysButLeaveCryptoState();
    StreamingSoftware* software;
    ClientState state;
    unsigned char authenticationKey[crypto_auth_KEYBYTES];
    unsigned char pullKey[crypto_secretstream_xchacha20poly1305_KEYBYTES];
    crypto_secretstream_xchacha20poly1305_state cryptoPullState;
    crypto_secretstream_xchacha20poly1305_state cryptoPushState;
};
