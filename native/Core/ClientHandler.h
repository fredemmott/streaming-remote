/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "ClientState.h"
#include "StreamingSoftware.h"

#include <sodium.h>
#include <nlohmann/json.hpp>

class MessageInterface;

class ClientHandler {
 public:
  explicit ClientHandler(
    StreamingSoftware* software,
    MessageInterface* connection);
  ~ClientHandler();

 private:
  void messageReceived(const std::string& message);

  void outputStateChanged(const std::string& id, OutputState state);
  void currentSceneChanged(const std::string& id);

  void handshakeClientHelloMessageReceived(const std::string& message);
  void handshakeClientReadyMessageReceived(const std::string& message);
  void encryptedRpcMessageReceived(const std::string& message);
  void plaintextRpcMessageReceived(const std::string& message);
  void encryptThenSendMessage(const std::string& message);
  void encryptThenSendMessage(const nlohmann::json& message);
  void cleanCrypto();
  void cleanCryptoKeysButLeaveCryptoState();
  StreamingSoftware* mSoftware;
  ClientState mState;
  MessageInterface* mConnection;
  unsigned char mAuthenticationKey[crypto_auth_KEYBYTES];
  unsigned char mPullKey[crypto_secretstream_xchacha20poly1305_KEYBYTES];
  crypto_secretstream_xchacha20poly1305_state mCryptoPullState;
  crypto_secretstream_xchacha20poly1305_state mCryptoPushState;
};
