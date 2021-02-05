/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Scene.h"

using json = nlohmann::json;

json Scene::toJson() const {
  return {
    {"id", id},
    {"name", name},
    {"active", active}
  };
}

Scene Scene::fromJson(const json& json) {
  return {
    .id = json["id"],
    .name = json["name"],
    .active = json["active"]
  };
}
