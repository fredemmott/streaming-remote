/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "Core/Logger.h"
#include "Core/Plugin.h"
#include "IXSplitScriptDllContext.h"
#include "XSplit.h"
#include "portability.h"

#ifdef WIN32
BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
#endif

namespace {
Plugin<XSplit>* sPlugin = nullptr;
}// namespace

extern "C" {

STREAMINGREMOTE_EXPORT
BOOL WINAPI XSplitScriptPluginInit() {
  LOG_FUNCTION();
  return true;
}

STREAMINGREMOTE_EXPORT
BOOL WINAPI XSplitScriptPluginCall(
  IXSplitScriptDllContext* pContext,
  BSTR functionName,
  BSTR* argv,
  UINT argc,
  BSTR* ret) {
  LOG_FUNCTION();
  Logger::debug("Context: {}", (void*)pContext);
  if (!sPlugin) {
    // XJS DLL Developer docs:$
    // - recommend initializing here instead of in the init function
    // - note that the context can be stored and re-used
    //
    // So, init here, and store the context as a member :)
    auto io_context = std::make_shared<asio::io_context>();
    sPlugin = new Plugin(io_context, new XSplit(io_context, pContext));
  }
  std::promise<bool> success;
  // Execute in the worker thread, but block on it succeeding
  asio::post(sPlugin->getContext(), [=, &success]() {
    success.set_value(sPlugin->getSoftware()->handleCall(
      pContext, functionName, argv, argc, ret));
  });
  auto f = success.get_future();
  Logger::debug("{}() - waiting for future", __FUNCTION__);
  f.wait();
  auto result = f.get();
  Logger::debug("{}() - future finished with result {}", __FUNCTION__, result);
  return result;
}

STREAMINGREMOTE_EXPORT
void WINAPI XSplitScriptPluginDestroy() {
  ScopeLogger _log(__FUNCTION__);
  delete sPlugin;
  sPlugin = nullptr;
}
}
