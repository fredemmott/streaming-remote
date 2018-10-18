/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Output.h"

#include <QJsonValue>

QString Output::typeToString(OutputType type) {
  switch (type) {
    case OutputType::UNKNOWN:
      return "unknown";
      break;
    case OutputType::LOCAL_RECORDING:
      return "local_recording";
      break;
    case OutputType::LOCAL_STREAM:
      return "local_stream";
      break;
    case OutputType::REMOTE_STREAM:
      return "remote_stream";
      break;
  }
}

OutputType Output::typeFromString(const QString& type) {
  if (type == "local_recording") {
    return OutputType::LOCAL_RECORDING;
  }
  if (type == "local_stream") {
    return OutputType::LOCAL_STREAM;
  }
  if (type == "remote_stream") {
    return OutputType::REMOTE_STREAM;
  }
  return OutputType::UNKNOWN;
}

OutputState Output::stateFromString(const QString& state) {
  if (state == "starting") {
    return OutputState::STARTING;
  }
  if (state == "active") {
    return OutputState::ACTIVE;
  }
  if (state == "stopping") {
    return OutputState::STOPPING;
  }
  if (state == "stopped") {
    return OutputState::STOPPED;
  }
  return OutputState::UNKNOWN;
}

QString Output::stateToString(OutputState state) {
    switch (state) {
      case OutputState::UNKNOWN:
        return "unknown";
        break;
      case OutputState::STARTING:
        return "starting";
        break;
      case OutputState::ACTIVE:
        return "active";
        break;
      case OutputState::STOPPING:
        return "stopping";
        break;
      case OutputState::STOPPED:
        return "stopped";
        break;
    }
  }


QJsonObject Output::toJson() const {
  QJsonObject obj {
    { "id", id },
    { "name", name },
    { "type", typeToString(type) },
    { "state", stateToString(state) }
  };
  if (delaySeconds >= 0) {
    obj["delaySeconds"] = delaySeconds;
  }
  return obj;
}

Output Output::fromJson(const QJsonObject& json) {
  Output ret;
  ret.id = json["id"].toString();
  ret.name = json["name"].toString();
  ret.type = typeFromString(json["type"].toString());
  ret.state = stateFromString(json["state"].toString());
  ret.delaySeconds = json.contains("delaySeconds") ? json["delaySeconds"].toInt() : -1;
  return ret;
}
