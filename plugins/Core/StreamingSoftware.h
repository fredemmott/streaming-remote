/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Config.h"
#include "Output.h"
#include "Signal.h"

#include <string>
#include <vector>

class StreamingSoftware {
 public:
  explicit StreamingSoftware();
  virtual ~StreamingSoftware();

  virtual Config getConfiguration() const = 0;
  virtual std::vector<Output> getOutputs() = 0;

  virtual void startOutput(const std::string& id) = 0;
  virtual void stopOutput(const std::string& id) = 0;
  virtual bool setOutputDelay(const std::string& id, int64_t seconds);

  Signal<const Config&> initialized;
  Signal<const std::string&, OutputState> outputStateChanged;
  Signal<const Config&> configurationChanged;
};
