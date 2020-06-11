/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Core/Config.h"
#include "Core/StreamingSoftware.h"

class Dummy : public StreamingSoftware {
 public:
  Dummy(const Config& config, const std::vector<Output>& outputs);
  ~Dummy();

  Config getConfiguration() const override;
  std::vector<Output> getOutputs() override;

  void startOutput(const std::string& id) override;
  void stopOutput(const std::string& id) override;

 private:
  Config mConfig;
  std::map<std::string, Output> mOutputs;
  void setOutputState(const std::string& id, OutputState state);
};
