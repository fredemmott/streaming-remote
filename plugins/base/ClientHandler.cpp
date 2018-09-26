/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "ClientHandler.h"

#include "StreamingSoftware.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <sodium.h>

#include <memory>

#define clean_and_return() \
    this->deleteLater(); \
    return;

#define clean_and_return_unless(x) \
  if (!(x)) { clean_and_return(); }

ClientHandler::ClientHandler(StreamingSoftware* parent)
: QObject(parent),
  software(parent),
  state(ClientState::UNINITIALIZED) {
  connect(parent, &StreamingSoftware::outputStateChanged, this, &ClientHandler::outputStateChanged);
}

ClientHandler::~ClientHandler() {
  cleanCrypto();
}

void ClientHandler::messageReceived(const QByteArray& message) {
  switch (state) {
    case ClientState::UNINITIALIZED:
      handshakeClientHelloMessageReceived(message);
      return;
    case ClientState::WAITING_FOR_CLIENT_READY:
      handshakeClientReadyMessageReceived(message);
      return;
    case ClientState::AUTHENTICATED:
      encryptedRpcMessageReceived(message);
      return;
  }
}

void ClientHandler::encryptedRpcMessageReceived(const QByteArray& c) {
  // No variable length arrays on MSVC :'(
  const size_t psize = c.size() - crypto_secretstream_xchacha20poly1305_ABYTES;
  auto p = std::unique_ptr<unsigned char>(new unsigned char[psize]);
  unsigned long long plen;
  unsigned char tag;
  const auto result = crypto_secretstream_xchacha20poly1305_pull(
    &this->cryptoPullState,
    p.get(), &plen,
    &tag,
    reinterpret_cast<const unsigned char*>(c.data()), c.size(),
    nullptr, 0
  );
  clean_and_return_unless(result == 0);
  assert(plen <= psize);
  plaintextRpcMessageReceived(QByteArray(
    reinterpret_cast<const char*>(p.get()),
    plen
  ));
}

void ClientHandler::plaintextRpcMessageReceived(const QByteArray& message) {
  auto doc = QJsonDocument::fromJson(message);
  if (doc.isNull()) {
    clean_and_return();
  }
  auto jsonrpc = doc.object();
  if (jsonrpc["jsonrpc"] != "2.0") {
    clean_and_return();
  }

  auto method = jsonrpc["method"].toString();

  if (method == "outputs/get") {
    const auto outputs = software->getOutputs();
    QJsonObject outputsJson;
    for (const auto& output: outputs) {
      outputsJson[output.id] = output.toJson();
    }

    QJsonDocument doc(QJsonObject {
      { "jsonrpc", "2.0" },
      { "id", jsonrpc["id"] },
      { "result", outputsJson }
    });
    encryptThenSendMessage(doc.toJson());
    return;
  }

  if (method == "outputs/start") {
    software->startOutput(jsonrpc["params"].toObject()["id"].toString());
    QJsonDocument doc(QJsonObject {
        { "jsonrpc", "2.0" },
        { "id", jsonrpc["id"] },
        { "result", QJsonObject {} }
    });
    encryptThenSendMessage(doc.toJson());
    return;
  }

  if (method == "outputs/stop") {
    software->stopOutput(jsonrpc["params"].toObject()["id"].toString());
    QJsonDocument doc(QJsonObject {
        { "jsonrpc", "2.0" },
        { "id", jsonrpc["id"] },
        { "result", QJsonObject {} }
    });
    encryptThenSendMessage(doc.toJson());
    return;
  }
}

namespace {
  #pragma pack(push, 1)
  struct ClientHelloBox {
    uint8_t serverToClientKey[crypto_secretstream_xchacha20poly1305_KEYBYTES];
  };
  struct ClientHelloMessage {
    uint8_t pwhashSalt[crypto_pwhash_SALTBYTES];
    uint8_t secretBoxNonce[crypto_secretbox_NONCEBYTES];
    uint8_t secretBox[sizeof(ClientHelloBox) + crypto_secretbox_MACBYTES];
  };
  struct ServerHelloBox {
    uint8_t clientToServerKey[crypto_secretstream_xchacha20poly1305_KEYBYTES];
    uint8_t authenticationKey[crypto_auth_KEYBYTES];
  };
  struct ServerHelloMessage {
    uint8_t secretBoxNonce[crypto_secretbox_NONCEBYTES];
    uint8_t secretBox[sizeof(ServerHelloBox) + crypto_secretbox_MACBYTES];
    uint8_t serverToClientHeader[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  };
  #pragma pack(pop)
}

void ClientHandler::handshakeClientHelloMessageReceived(const QByteArray& blob) {
  clean_and_return_unless(blob.size() == sizeof(ClientHelloMessage));
  const ClientHelloMessage* request =
    reinterpret_cast<const ClientHelloMessage*>(blob.data());
  ClientHelloBox requestBox;
  ServerHelloBox responseBox;
  ServerHelloMessage response;

  // Open the box!
  const auto password = software->getConfiguration().password.toUtf8();
  uint8_t psk[crypto_secretbox_KEYBYTES];
  {
    const auto result = crypto_pwhash(
      psk, crypto_secretbox_KEYBYTES,
      password.data(), password.size(),
      request->pwhashSalt,
      crypto_pwhash_OPSLIMIT_INTERACTIVE,
      crypto_pwhash_MEMLIMIT_INTERACTIVE,
      crypto_pwhash_ALG_DEFAULT
    );
    clean_and_return_unless(result == 0);
  }
  {
    const auto result = crypto_secretbox_open_easy(
      reinterpret_cast<uint8_t*>(&requestBox),
      request->secretBox, sizeof(request->secretBox),
      request->secretBoxNonce,
      psk
    );
    clean_and_return_unless(result == 0);
  }

  // Process and respond
  {
    const auto result = crypto_secretstream_xchacha20poly1305_init_push(
      &this->cryptoPushState,
      response.serverToClientHeader,
      requestBox.serverToClientKey
    );
    clean_and_return_unless(result == 0);
  }

  crypto_secretstream_xchacha20poly1305_keygen(responseBox.clientToServerKey);
  crypto_auth_keygen(responseBox.authenticationKey);
  static_assert(
    sizeof(responseBox.clientToServerKey) == sizeof(this->pullKey),
    "differenting pull key sizes"
  );
  memcpy(
    this->pullKey,
    responseBox.clientToServerKey,
    sizeof(responseBox.clientToServerKey)
  );
  static_assert(
    sizeof(responseBox.authenticationKey) == sizeof(this->authenticationKey),
    "differing authentication key sizes"
  );
  memcpy(
    this->authenticationKey,
    responseBox.authenticationKey,
    sizeof(responseBox.authenticationKey)
  );

  randombytes_buf(response.secretBoxNonce, sizeof(response.secretBoxNonce));
  {
    const auto result = crypto_secretbox_easy(
      response.secretBox,
      reinterpret_cast<const uint8_t*>(&responseBox), sizeof(responseBox),
      response.secretBoxNonce,
      psk
    );
    clean_and_return_unless(result == 0);
  }
  this->state = ClientState::WAITING_FOR_CLIENT_READY;
  emit sendMessage(QByteArray(reinterpret_cast<const char*>(&response), sizeof(response)));
}

void ClientHandler::outputStateChanged(const QString& id, OutputState state) {
  QJsonDocument doc(QJsonObject {
    { "jsonrpc", "2.0" },
    { "method", "outputs/stateChanged" },
    { "params", QJsonObject {
      { "id", id },
      { "state", Output::stateToString(state) }
    }}
  });
  encryptThenSendMessage(doc.toJson());
}

namespace {
  #pragma pack(push, 1)
  struct ClientReadyMessage {
    uint8_t clientToServerHeader[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    uint8_t authenticationMac[crypto_auth_BYTES];
  };
  #pragma pack(pop)
}

void ClientHandler::handshakeClientReadyMessageReceived(
  const QByteArray& blob
) {
  clean_and_return_unless(blob.size() == sizeof(ClientReadyMessage));
  const ClientReadyMessage* request =
    reinterpret_cast<const ClientReadyMessage*>(blob.data());
  {
    const int result = crypto_auth_verify(
      request->authenticationMac,
      request->clientToServerHeader, sizeof(request->clientToServerHeader),
      this->authenticationKey
    );
    clean_and_return_unless(result == 0);
  }
  {
    const int result = crypto_secretstream_xchacha20poly1305_init_pull(
      &this->cryptoPullState,
      request->clientToServerHeader,
      this->pullKey
    );
    clean_and_return_unless(result == 0);
  }

  this->state = ClientState::AUTHENTICATED;

  const QJsonDocument json(QJsonObject {
    { "jsonrpc", "2.0" },
    { "method", "hello" }
  });
  this->encryptThenSendMessage(json.toJson());
}

void ClientHandler::cleanCrypto() {
  sodium_memzero(&this->cryptoPullState, sizeof(this->cryptoPullState));
  sodium_memzero(&this->cryptoPushState, sizeof(this->cryptoPushState));
}

void ClientHandler::encryptThenSendMessage(const QByteArray& p) {
  if (this->state != ClientState::AUTHENTICATED) {
    return;
  }

  auto csize = p.size() + crypto_secretstream_xchacha20poly1305_ABYTES;
  auto c = std::unique_ptr<unsigned char>(new unsigned char[csize]);
  unsigned long long clen;
  const auto result = crypto_secretstream_xchacha20poly1305_push(
    &this->cryptoPushState,
    c.get(), &clen,
    reinterpret_cast<const unsigned char*>(p.data()), p.size(),
    nullptr, 0, 0
  );
  assert(clen <= csize);
  clean_and_return_unless(result == 0);
  emit sendMessage(QByteArray(
    reinterpret_cast<const char*>(c.get()),
    clen
  ));
}
