/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Config.h"

#include <sodium.h>

namespace {
  QString s_defaultPassword;
}

Config Config::getDefault() {
  if (s_defaultPassword.isNull()) {
    uint8_t buf[8];
    randombytes_buf(buf, sizeof(buf));
    s_defaultPassword = QByteArray(
      reinterpret_cast<const char*>(buf),
      sizeof(buf)
    ).toHex();
    sodium_memzero(buf, sizeof(buf));
  }

  return Config {
    s_defaultPassword, // password
    QString(), // local socket
    9001, // tcp port
    9002 // websocket port
  };
};
