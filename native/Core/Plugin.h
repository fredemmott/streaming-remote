/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <asio.hpp>
#include <future>
#include <thread>
#include <type_traits>

#include "Logger.h"
#include "Plugin.h"
#include "Server.h"
#include "StreamingSoftware.h"

class StreamingSoftware;

template <class T>
class Plugin final {
  static_assert(
    std::is_base_of<StreamingSoftware, T>::value,
    "T must be StreamingSoftware");

 public:
  Plugin(T* software)
    : mContext(), mWork(asio::make_work_guard(mContext)), mSoftware(software) {
    ScopeLogger log_("{}()", __FUNCTION__);
    init();
  }

  Plugin()
    : mContext(), mWork(asio::make_work_guard(mContext)), mSoftware(new T()) {
    LOG_FUNCTION();
    init();
  }

  ~Plugin() {
    LOG_FUNCTION();
    mWork.reset();
    mContext.stop();
    Logger::debug("~Plugin: waiting for thread");
    wait();
    Logger::debug("~Plugin: thread joined");
    delete mSoftware;
    mSoftware = nullptr;
  }

  asio::io_context& getContext() {
    return mContext;
  }

  void wait() {
    mThread.join();
  }

  T* getSoftware() {
    return mSoftware;
  }

 private:
  void init() {
    std::promise<void> running;
    asio::post(mContext, [&running] { running.set_value(); });
    mThread = std::thread([this]() {
      auto server = new Server(&mContext, this->mSoftware);
      if (mSoftware->initialized.wasEmitted()) {
        server->startListening(mSoftware->getConfiguration());
      } else {
        mSoftware->initialized.connect(server, &Server::startListening);
      }
      Logger::debug("Starting ASIO context");
      try {
        mContext.run();
        Logger::debug("ASIO context cleanly finished");
      } catch (...) {
        Logger::debug("Unclean exit from ASIO context");
        throw;
      }
      delete server;
    });
    auto future = running.get_future();
    future.wait();
  }

  std::thread mThread;
  asio::io_context mContext;
  asio::executor_work_guard<asio::io_context::executor_type> mWork;
  T* mSoftware;
};
