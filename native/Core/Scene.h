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

struct Scene {
  std::string id;
  std::string name;

  nlohmann::json toJson() const;
  static Scene fromJson(const nlohmann::json&);
};
