/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "Core/Logger.h"
#include "Core/StreamingSoftware.h"

#include <obs-frontend-api.h>
#include <QObject>

class OBS : public QObject, public StreamingSoftware {
  Q_OBJECT
 public:
  OBS();
  ~OBS();

  std::vector<Output> getOutputs();
  Config getConfiguration() const;

 public:
  void startOutput(const std::string& id);
  void stopOutput(const std::string& id);
  bool setOutputDelay(const std::string& id, int64_t seconds);

 private:
  Config getInitialConfiguration();
  void setConfiguration(const Config& config);

  void dispatchFrontendEvent(enum obs_frontend_event event);

  static void frontendEventCallback(enum obs_frontend_event event, void* data);

  Config mConfig;
  Logger::ImplRegistration mLoggerImpl;
};
