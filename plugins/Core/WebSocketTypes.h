/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace {
typedef websocketpp::config::asio WebSocketConfig;
}

typedef websocketpp::server<WebSocketConfig> WebSocketServerImpl;
