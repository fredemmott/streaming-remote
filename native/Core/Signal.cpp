/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Signal.h"
#include "Logger.h"

ConnectionImplBase::~ConnectionImplBase() {
}

ScopedConnection::ScopedConnection(Connection c): mConnection(std::move(c)) {
}

ScopedConnection::~ScopedConnection() {
  if (mConnection) {
    // May be invalid if moved
    mConnection->disconnect();
  }
}

ConnectionOwner::~ConnectionOwner() {
}
