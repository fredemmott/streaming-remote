/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Config.h"

#include <sodium.h>

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "Logger.h"

namespace {
std::string s_defaultPassword;

std::string to_hex(const std::string& other) {
  std::string out;
  out.reserve(other.size() * 2);
  for (const char c : other) {
    out += fmt::format("{:02x}", (uint8_t)c);
  }
  return out;
}
}// namespace

Config Config::getDefault() {
  if (s_defaultPassword.empty()) {
    uint8_t buf[8];
    randombytes_buf(buf, sizeof(buf));
    s_defaultPassword
      = to_hex(std::string(reinterpret_cast<const char*>(buf), sizeof(buf)));
    sodium_memzero(buf, sizeof(buf));
  }

  return Config{
    .password = s_defaultPassword,
    .tcpPort = 9001,
    .webSocketPort = 9002
  };
};
