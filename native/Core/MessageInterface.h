/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Signal.h"

#include <string>

class MessageInterface {
 public:
  virtual ~MessageInterface();
  virtual void sendMessage(const std::string& message) = 0;
  virtual void disconnect() = 0;
  Signal<const std::string&> messageReceived;
  Signal<> disconnected;

 protected:
  MessageInterface();
};
