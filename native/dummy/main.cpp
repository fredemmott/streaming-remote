/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Core/Config.h"
#include "Core/Output.h"
#include "Core/Plugin.h"
#include "Dummy.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  // clang-format off
  const Config config {
    .password = "hello, world",
    .tcpPort = 9001,
    .webSocketPort = 9002
  };
  const std::vector<Output> outputs {
    {
      .id = "record_id",
      .name = "Record",
      .state = OutputState::STOPPED,
      .type = OutputType::LOCAL_RECORDING,
    },
    {
      .id = "stream_id",
      .name = "Stream",
      .state = OutputState::STOPPED,
      .type = OutputType::REMOTE_STREAM,
    }
  };
  // clang-format on
  Plugin<Dummy> plugin(new Dummy(config, outputs));
  cout << "Started server with password '" << config.password << "'..." << endl;
  plugin.wait();
  return 0;
}
