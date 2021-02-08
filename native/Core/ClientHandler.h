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

#include <asio/awaitable.hpp>
#include <sodium.h>
#include <nlohmann/json.hpp>

class MessageInterface;

namespace asio {
class io_context;
}

class ClientHandler : private ConnectionOwner {
 public:
  explicit ClientHandler(
    std::shared_ptr<asio::io_context> context,
    StreamingSoftware* software,
    MessageInterface* connection);
  ~ClientHandler();

 private:
  asio::awaitable<void> messageReceived(const std::string message);

  void outputStateChanged(const std::string& id, OutputState state);
  void currentSceneChanged(const std::string& id);

  void handshakeClientHelloMessageReceived(const std::string& message);
  void handshakeClientReadyMessageReceived(const std::string& message);
  asio::awaitable<void> encryptedRpcMessageReceived(const std::string& message);
  asio::awaitable<void> plaintextRpcMessageReceived(const std::string& message);
  void encryptThenSendMessage(const std::string& message);
  void encryptThenSendMessage(const nlohmann::json& message);
  void cleanCrypto();
  void cleanCryptoKeysButLeaveCryptoState();

  ClientState mState;
  std::shared_ptr<asio::io_context> mIoContext;
  StreamingSoftware* mSoftware;
  MessageInterface* mConnection;
  unsigned char mAuthenticationKey[crypto_auth_KEYBYTES];
  unsigned char mPullKey[crypto_secretstream_xchacha20poly1305_KEYBYTES];
  crypto_secretstream_xchacha20poly1305_state mCryptoPullState;
  crypto_secretstream_xchacha20poly1305_state mCryptoPushState;
};
