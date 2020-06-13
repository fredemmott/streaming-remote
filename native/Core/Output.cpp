/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Output.h"

using json = nlohmann::json;

std::string Output::typeToString(OutputType type) {
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

OutputType Output::typeFromString(const std::string& type) {
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

OutputState Output::stateFromString(const std::string& state) {
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

std::string Output::stateToString(OutputState state) {
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

json Output::toJson() const {
  json obj{{"id", id},
           {"name", name},
           {"type", typeToString(type)},
           {"state", stateToString(state)}};
  if (delaySeconds >= 0) {
    obj["delaySeconds"] = static_cast<int64_t>(delaySeconds);
  }
  return obj;
}

Output Output::fromJson(const json& json) {
  return {.id = json["id"],
          .name = json["name"],
          .state = stateFromString(json["state"]),
          .type = typeFromString(json["type"]),
          .delaySeconds = json.find("delaySeconds") != json.end()
                            ? int(json["delaySeconds"])
                            : -1};
}
