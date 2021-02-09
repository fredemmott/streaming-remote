/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Plugin.h"

#include "Logger.h"
#include "Server.h"
#include "StreamingSoftware.h"

Plugin::Plugin(
  std::shared_ptr<asio::io_context> context,
  std::shared_ptr<StreamingSoftware> software
):
  mContext(context),
  mWork(asio::make_work_guard(*mContext)),
  mSoftware(software)
{
  LOG_FUNCTION();

  std::promise<void> running;
  asio::post(*mContext, [&running] { running.set_value(); });
  mThread = std::thread([this]() {
    auto server = new Server(mContext, this->mSoftware);
    if (mSoftware->initialized.wasEmitted()) {
      server->startListening(mSoftware->getConfiguration());
    } else {
      mSoftware->initialized.connect(server, &Server::startListening);
    }
    Logger::debug("Starting ASIO context");
    try {
      mContext->run();
      Logger::debug("ASIO context cleanly finished");
    } catch (const std::exception& e) {
      Logger::debug("Unclean exit from ASIO context: {}", e.what());
      throw;
    } catch (...) {
      Logger::debug("Unclean exit from ASIO context");
      throw;
    }
    delete server;
  });
  auto future = running.get_future();
  future.wait();
}

Plugin::~Plugin() {
  LOG_FUNCTION();
  mWork.reset();
  mContext->stop();
  Logger::debug("~Plugin: waiting for thread");
  wait();
  Logger::debug("~Plugin: thread joined");
}

asio::io_context& Plugin::getContext() {
  return *mContext;
}

void Plugin::wait() {
  mThread.join();
}
