/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <asio.hpp>
#include <functional>
#include <map>

#include "Core/Config.h"
#include "Core/Logger.h"
#include "Core/Signal.h"
#include "Core/StreamingSoftware.h"
#include "IXSplitScriptDllContext.h"
#include "portability.h"

class XSplit final : public StreamingSoftware {
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
  Config mConfig;
  CComPtr<IXSplitScriptDllContext> mCallbackImpl;
  std::vector<Output> mOutputs;
  Logger::ImplRegistration mLoggerImpl;
  std::map<std::string, std::function<void(BSTR*, UINT, BSTR*)>> mPluginFuncs;

  void sendToXSplitDebugLog(const std::string&);
  void setJsonConfig(const nlohmann::json& jsonConfig);

  template <class... Targs>
  void callJSPlugin(const char* func, Targs... args) {
    BSTR com_func = NEW_BSTR_FROM_STDSTRING(
      // ideally:
      //   fmt::format("com.fredemmott.streaming-remote/js/{}", func));
      // ... but it needs to be a valid JS function name; `OnDll<foo>()` is
      // called by XSplit.
      fmt::format("com_fredemmott_streamingremote__js__{}", func));
    std::vector<std::string> argv{args...};
    BSTR* com_args = (BSTR*)malloc(sizeof(BSTR) * sizeof...(Targs));
    for (size_t i = 0; i < sizeof...(Targs); ++i) {
      com_args[i] = NEW_BSTR_FROM_STDSTRING(argv[i]);
    }
    mCallbackImpl->Callback(com_func, com_args, sizeof...(Targs));
  }

  void pluginfunc_init(const std::string& proto_version);
  void pluginfunc_setConfig(
    const nlohmann::json& config,
    const nlohmann::json& outputs);
  void pluginfunc_outputStateChanged(
    const std::string& id,
    const std::string& state);
  nlohmann::json pluginfunc_getDefaultConfiguration();
  void pluginfunc_setConfiguration(const nlohmann::json& config);

  /////////////////////////////////////////////////
  //// THERE BE C++17 DRAGONS BEYOND THIS LINE ////
  /////////////////////////////////////////////////

  /* Magic stuff for converting JS->CPP calls:
   * - XSplit calls handleFunc("foo", ret, 2, {"bar", "baz"});
   * - we call `*ret = pluginfunc_foo("bar", "baz")
   *
   * And automatically...
   * - verify argc
   * - convert to/from nlohmann::json - funcs don't need to take/return strings
   * - handle void returns
   *
   * User API:
   *
   *   registerPluginFunc("foo", &XSplit::pluginfunc_foo)
   */

  template <class TF, size_t... Tis>
  static auto make_mapped_tuple(TF mapper, std::index_sequence<Tis...>) {
    return std::make_tuple(mapper(Tis)...);
  }

  template <class TRet, class... Targs>
  auto call_plugin_func(
    BSTR* ret,
    TRet (XSplit::*impl)(const Targs&...),
    Targs... args) {
    *ret = NEW_BSTR_FROM_STDSTRING((this->*impl)(args...));
  }

  template <class... Targs>
  auto call_plugin_func(
    BSTR* ret,
    void (XSplit::*impl)(const Targs&...),
    Targs... args) {
    (this->*impl)(args...);
  }

  template <class TRet, class... Targs>
  void registerPluginFunc(
    const char* name,
    TRet (XSplit::*impl)(const Targs&...)) {
    mPluginFuncs[fmt::format("com.fredemmott.streaming-remote/cpp/{}", name)] =
      [=](BSTR* ret, UINT argc, BSTR* bargv) {
        if (argc != sizeof...(Targs)) {
          Logger::debug(
            "{}() expected {} args, got {}", name, sizeof...(Targs), argc);
          return;
        }
        auto tuple = make_mapped_tuple(
          [bargv](size_t i) { return STDSTRING_FROM_BSTR(bargv[i]); },
          std::make_index_sequence<sizeof...(Targs)>{});
        std::apply(
          [=](Targs... args) { call_plugin_func(ret, impl, args...); }, tuple);
      };
  }
};
