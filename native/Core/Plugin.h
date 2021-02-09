/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <asio.hpp>
#include <thread>

class StreamingSoftware;

class Plugin final {
 public:
  Plugin(
    std::shared_ptr<asio::io_context> context,
    std::shared_ptr<StreamingSoftware> software
  );
  ~Plugin();

  asio::io_context& getContext();

  void wait();

 private:
  std::thread mThread;
  std::shared_ptr<asio::io_context> mContext;
  std::shared_ptr<StreamingSoftware> mSoftware;
  asio::executor_work_guard<asio::io_context::executor_type> mWork;
};
