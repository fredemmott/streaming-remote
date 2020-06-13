/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <fmt/format.h>

#include <string>

class Logger {
 public:
  static void debug(const std::string& in) {
    logImpl(in);
  }

  template <typename T, typename... Args>
  static void debug(const T& fmt, Args&&... args) {
    logImpl(fmt::format(fmt, args...));
  }

  typedef std::function<void(const std::string&)> Impl;
  class ImplRegistration {
   public:
    ImplRegistration() = delete;
    ImplRegistration(const Impl& impl);
    ImplRegistration(const ImplRegistration&) = delete;
    ImplRegistration(ImplRegistration&&);
    ~ImplRegistration();
    void release();

   private:
    size_t mId;
  };

 private:
  static void logImpl(const std::string& message);
};

class ScopeLogger {
 public:
  ScopeLogger() = delete;
  ScopeLogger(const std::string& message) {
    init(message);
  }

  template <typename T, typename... Args>
  ScopeLogger(const T& fmt, Args&&... args) {
    init(fmt::format(fmt, args...));
  }

  ScopeLogger(const ScopeLogger& other) = delete;

  ~ScopeLogger() {
    Logger::debug("{} - EXIT", mMessage);
  }

 private:
  void init(const std::string& message) {
    Logger::debug("{} - ENTER", message);
    mMessage = message;
  }
  std::string mMessage;
};

// TODO: support logging function arguments
#define LOG_FUNCTION(...) ScopeLogger _function_scope_log("{}()", __FUNCTION__)
