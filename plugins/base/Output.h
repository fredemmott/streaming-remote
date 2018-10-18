/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <QJsonObject>
#include <QString>

enum class OutputState {
  UNKNOWN,
  STARTING,
  ACTIVE,
  STOPPING,
  STOPPED
};

enum class OutputType {
  UNKNOWN,
  LOCAL_RECORDING,
  LOCAL_STREAM, // e.g. NDI
  REMOTE_STREAM
};

struct Output {
  QString id;
  QString name;
  OutputState state;
  OutputType type;

  int64_t delaySeconds;

  QJsonObject toJson() const;
  static Output fromJson(const QJsonObject&);

  static QString typeToString(OutputType);
  static OutputType typeFromString(const QString&);
  static QString stateToString(OutputState);
  static OutputState stateFromString(const QString&);
};
