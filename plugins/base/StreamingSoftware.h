/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <QObject>
#include "Config.h"
#include "Output.h"

class StreamingSoftware : public QObject {
  Q_OBJECT

 public:
  explicit StreamingSoftware(QObject* parent = nullptr);
  virtual ~StreamingSoftware();

  virtual Config getConfiguration() const = 0;
  virtual QList<Output> getOutputs() = 0;
 public slots:
  virtual void startOutput(const QString& id) = 0;
  virtual void stopOutput(const QString& id) = 0;
  virtual bool setOutputDelay(const QString& id, int64_t seconds);
 signals:
  void outputStateChanged(const QString& id, OutputState newState);
  void configurationChanged(const Config& config);
};
