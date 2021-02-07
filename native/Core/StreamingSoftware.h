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
#include "Scene.h"
#include "Signal.h"

#include <asio/awaitable.hpp>

#include <memory>
#include <string>
#include <vector>

class StreamingSoftware {
 public:
  explicit StreamingSoftware(std::shared_ptr<asio::io_context> mContext);
  virtual ~StreamingSoftware();

  virtual Config getConfiguration() const = 0;

  virtual asio::awaitable<std::vector<Output>> getOutputs() = 0;

  virtual void startOutput(const std::string& id) = 0;
  virtual void stopOutput(const std::string& id) = 0;
  virtual bool setOutputDelay(const std::string& id, int64_t seconds);

  virtual asio::awaitable<std::vector<Scene>> getScenes();
  virtual asio::awaitable<bool> activateScene(const std::string& id);

  Signal<const Config&> initialized;
  Signal<const Config&> configurationChanged;
  Signal<const std::string&, OutputState> outputStateChanged;
  Signal<const std::string&> currentSceneChanged;
 protected:
  asio::io_context& getIoContext() const noexcept;
 private:
  std::shared_ptr<asio::io_context> mContext;
};
