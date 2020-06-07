/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include <obs-module.h>

#include "base/SocketServer.h"
#include "obs/OBS.h"

extern "C" {
OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Fred Emmott")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streaming-remote", "en-US");

bool obs_module_load() {
  (new SocketServer(new OBS()))->startListening();
  return true;
}

const char* obs_module_name() {
  return "streaming-remote";
}

const char* obs_module_description() {
  return "Remote control of OBS via sockets and websockets";
}
}
