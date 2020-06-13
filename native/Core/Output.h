/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>

enum class OutputState { UNKNOWN, STARTING, ACTIVE, STOPPING, STOPPED };

enum class OutputType {
  UNKNOWN,
  LOCAL_RECORDING,
  LOCAL_STREAM,// e.g. NDI
  REMOTE_STREAM
};

struct Output {
  std::string id;
  std::string name;
  OutputState state = OutputState::UNKNOWN;
  OutputType type = OutputType::UNKNOWN;

  int64_t delaySeconds = -1;

  nlohmann::json toJson() const;
  static Output fromJson(const nlohmann::json&);

  static std::string typeToString(OutputType);
  static OutputType typeFromString(const std::string&);
  static std::string stateToString(OutputState);
  static OutputState stateFromString(const std::string&);
};
