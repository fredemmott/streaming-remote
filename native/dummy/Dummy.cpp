/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Dummy.h"

#include "Core/Config.h"

#include <iostream>

using namespace std;

Dummy::Dummy(
  std::shared_ptr<asio::io_context> ctx,
  const Config& config,
  const std::vector<Output>& outputs
): StreamingSoftware(ctx), mConfig(config) {
  for (const auto& output : outputs) {
    mOutputs[output.id] = output;
  }
  emit initialized(config);
}

Dummy::~Dummy() {
}

Config Dummy::getConfiguration() const {
  return mConfig;
}

asio::awaitable<std::vector<Output>> Dummy::getOutputs() {
  std::vector<Output> ret;
  ret.reserve(mOutputs.size());
  for (const auto& [id, output] : mOutputs) {
    ret.push_back(output);
  }
  co_return ret;
}

asio::awaitable<void> Dummy::startOutput(const std::string& id) {
  cout << "Starting output " << id << endl;
  setOutputState(id, OutputState::STARTING);
  setOutputState(id, OutputState::ACTIVE);
  co_return;
}

asio::awaitable<void> Dummy::stopOutput(const std::string& id) {
  cout << "stopping output: " + id << endl;
  setOutputState(id, OutputState::STOPPING);
  setOutputState(id, OutputState::STOPPED);
  co_return;
}

void Dummy::setOutputState(const std::string& id, OutputState state) {
  mOutputs[id].state = state;
  emit outputStateChanged(id, state);
}
