/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <asio.hpp>

#include "Core/Config.h"
#include "Core/Logger.h"
#include "Core/Signal.h"
#include "Core/StreamingSoftware.h"
#include "IXSplitScriptDllContext.h"
#include "portability.h"

class XSplit : public StreamingSoftware {
 public:
  XSplit(IXSplitScriptDllContext* context);
  ~XSplit();

  Config getConfiguration() const override;
  std::vector<Output> getOutputs() override;

  bool handleCall(
    IXSplitScriptDllContext* context,
    BSTR functionName,
    BSTR* argv,
    UINT argc,
    BSTR* retv);

  // slots:
  void startOutput(const std::string& id) override;
  void stopOutput(const std::string& id) override;

 private:
  void sendToXSplitDebugLog(const std::string&);
  void setJsonConfig(const nlohmann::json& jsonConfig);

  Config mConfig;
  CComPtr<IXSplitScriptDllContext> mCallbackImpl;
  std::vector<Output> mOutputs;
  Logger::ImplRegistration mLoggerImpl;

  template <class... Targs>
  void callJSPlugin(const char* func, Targs... args) {
    OutputDebugStringA("[xsplit-streaming-remote] Calling JS");
    BSTR com_func = NEW_BSTR_FROM_STDSTRING(
      // ideally:
      //   fmt::format("com.fredemmott.streaming-remote/js/{}", func));
      // ... but it needs to be a valid JS function name; `OnDll<foo>()` is
      // called by XSplit.
      fmt::format("com_fredemmott_streamingremote__js__{}", func));
    std::vector<std::string> argv{args...};
    asio::post([=]() {
      BSTR* com_args = (BSTR*)malloc(sizeof(BSTR) * sizeof...(Targs));
      for (size_t i = 0; i < sizeof...(Targs); ++i) {
        com_args[i] = NEW_BSTR_FROM_STDSTRING(argv[i]);
      }
      mCallbackImpl->Callback(com_func, com_args, sizeof...(Targs));
    });
  }
};
