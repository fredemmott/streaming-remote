/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "StreamingSoftware.h"

StreamingSoftware::StreamingSoftware(
  std::shared_ptr<asio::io_context> context
): mContext(context) {
}

StreamingSoftware::~StreamingSoftware() {
}

bool StreamingSoftware::setOutputDelay(
  const std::string& name,
  int64_t seconds) {
  return false;
}

asio::awaitable<std::vector<Scene>> StreamingSoftware::getScenes() {
  co_return std::vector<Scene>();
}

asio::awaitable<bool> StreamingSoftware::activateScene(const std::string& id) {
  co_return false;
}

asio::io_context& StreamingSoftware::getIoContext() const noexcept {
  return *mContext;
}
