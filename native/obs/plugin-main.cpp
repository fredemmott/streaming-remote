/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include <obs-module.h>

#include "Core/Logger.h"
#include "Core/Plugin.h"
#include "OBS.h"

namespace {
Plugin* sPlugin = nullptr;
}

extern "C" {
OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Fred Emmott")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streaming-remote", "en-US");

bool obs_module_load() {
  LOG_FUNCTION();
  auto ctx = std::make_shared<asio::io_context>();
  sPlugin = new Plugin(ctx, new OBS(ctx));
  return true;
}

void obs_module_unload() {
  LOG_FUNCTION();
  delete sPlugin;
  sPlugin = nullptr;
}

const char* obs_module_name() {
  return "streaming-remote";
}

const char* obs_module_description() {
  return "Remote control of OBS via sockets and websockets";
}
}
