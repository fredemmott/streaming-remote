/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "base/StreamingSoftware.h"

#include <obs-frontend-api.h>

class OBS : public StreamingSoftware {
  Q_OBJECT;

 public:
  OBS(QObject* parent = nullptr);
  ~OBS();

  QList<Output> getOutputs();
  Config getConfiguration() const;
 public slots:
  void startOutput(const QString& id);
  void stopOutput(const QString& id);
  bool setOutputDelay(const QString& id, int64_t seconds);

 private:
  Config getInitialConfiguration();
  void setConfiguration(const Config& config);

  void dispatchFrontendEvent(enum obs_frontend_event event);

  static void frontendEventCallback(enum obs_frontend_event event, void* data);

  Config config;
};
