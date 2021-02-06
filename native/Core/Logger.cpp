/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Logger.h"

#include <exception>
#include <map>
#include <mutex>

#ifdef _MSC_VER
#include "Windows.h"
#endif

namespace {

#if __cplusplus > 201703L
inline bool uncaught_exception() noexcept {
	return std::uncaught_exceptions() > 0;
}
#else
using std::uncaught_exception;
#endif

void native_logger(const std::string& message) {
#ifndef NDEBUG
#ifdef _MSC_VER
  HMODULE this_dll = nullptr;
  GetModuleHandleExA(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    reinterpret_cast<LPCSTR>(&native_logger), &this_dll);
  char name[1024];
  memset(name, 0, sizeof(name));
  GetModuleFileNameA(this_dll, name, sizeof(name));
  const char* basename = strrchr(name, '\\');
  auto expanded
    = fmt::format("[{}] {}", basename ? basename + 1 : name, message);
  OutputDebugStringA(expanded.c_str());
#endif
#endif
}

std::recursive_mutex sMutex;
std::map<size_t, Logger::Impl> sLoggers{{0, &native_logger}};
size_t sNextLogger = 1;
}// namespace

Logger::ImplRegistration::ImplRegistration(const Logger::Impl& impl)
  : mId(sNextLogger++) {
  {
    std::scoped_lock lock(sMutex);
    sLoggers[mId] = impl;
  }
}
Logger::ImplRegistration::ImplRegistration(ImplRegistration&& other)
  : mId(other.mId) {
  other.mId = 0;
}

Logger::ImplRegistration::~ImplRegistration() {
  release();
}

void Logger::ImplRegistration::release() {
  if (mId == 0) {
    return;

  }
  std::scoped_lock lock(sMutex);
  sLoggers.erase(mId);
  mId = 0;
}

void Logger::logImpl(const std::string& message) {
  std::scoped_lock lock(sMutex);
  for (const auto& [_, impl] : sLoggers) {
    impl(message);
  }
}

ScopeLogger::~ScopeLogger() {
  if (uncaught_exception()) {
    Logger::debug("{} - EXCEPTION", mMessage);
  } else {
    Logger::debug("{} - EXIT", mMessage);
  }
}
