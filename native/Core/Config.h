/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <string>

struct Config {
  std::string password;
  std::string localSocket;
  uint16_t tcpPort;
  uint16_t webSocketPort;

  static Config getDefault();
};
