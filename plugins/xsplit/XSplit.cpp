/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "XSplit.h"

#include <fmt/format.h>

#include <set>
#include <thread>

#include "Core/Config.h"
#include "Core/Logger.h"

#define DebugPrint(...) Logger::debug(__VA_ARGS__)

using json = nlohmann::json;

#define CHECK(x) \
  if (!(x)) { \
    DebugPrint("Assertion failed at {}:{}: {}" __FILE__, __LINE__, #x); \
    return false; \
  }

XSplit::XSplit(IXSplitScriptDllContext* context)
  : StreamingSoftware(),
    mCallbackImpl(context),
    mLoggerImpl([this](const std::string& message) {
      this->sendToXSplitDebugLog(message);
    }) {
  LOG_FUNCTION();
}

XSplit::~XSplit() {
  LOG_FUNCTION();
}

Config XSplit::getConfiguration() const {
  LOG_FUNCTION();
  return mConfig;
}

std::vector<Output> XSplit::getOutputs() {
  LOG_FUNCTION();
  return mOutputs;
}

void XSplit::setJsonConfig(const nlohmann::json& doc) {
  mConfig.password = doc["password"].is_null() ? "" : doc["password"];
  mConfig.localSocket = doc["localSocket"].is_null() ? "" : doc["localSocket"];
  mConfig.tcpPort = doc["tcpPort"];
  mConfig.webSocketPort = doc["webSocketPort"];
}

bool XSplit::handleCall(
  IXSplitScriptDllContext* context,
  BSTR wideFunctionName,
  BSTR* bargv,
  UINT argc,
  BSTR* retv) {
  LOG_FUNCTION();
  if (context != mCallbackImpl) {
    mCallbackImpl = context;
  }

  const auto fun = STDSTRING_FROM_BSTR(wideFunctionName);
  std::vector<std::string> argv;
  DebugPrint("Call: {}", fun);
  for (UINT i = 0; i < argc; ++i) {
    const auto arg = STDSTRING_FROM_BSTR(bargv[i]);
    DebugPrint("- argv[{}]: {}", i, arg);
    argv.push_back(arg);
  }

  if (fun == "StreamingRemote.init") {
    CHECK(argc == 2);
    const auto config = json::parse(argv[0]);
    const auto outputs = json::parse(argv[1]);
    DebugPrint("Setting config to {}", config.dump());
    setJsonConfig(config);

    DebugPrint("Setting outputs to {}", outputs.dump());
    mOutputs.clear();
    for (const auto& output : outputs) {
      mOutputs.push_back(Output::fromJson(output));
    }
    emit initialized(mConfig);
    return true;
  }

  if (fun == "StreamingRemote.outputStateChanged") {
    CHECK(argc == 2);
    const auto id = argv[0];
    const auto stateStr = argv[1];
    const auto state = Output::stateFromString(stateStr);
    DebugPrint(
      "State changed: {} => {} ({})", id, stateStr,
      Output::stateToString(state));
    for (auto& output : mOutputs) {
      if (output.id == id) {
        output.state = state;
        break;
      }
    }
    emit outputStateChanged(id, state);
    return true;
  }

  if (fun == "StreamingRemote.getDefaultConfiguration") {
    CHECK(argc == 0);
    auto config = Config::getDefault();
    json doc({{"password", config.password},
              {"localSocket", config.localSocket},
              {"tcpPort", config.tcpPort},
              {"webSocketPort", config.webSocketPort}});
    DebugPrint("Returning default config: {}", doc.dump());
    *retv = NEW_BSTR_FROM_STDSTRING(doc.dump());
    return true;
  }

  if (fun == "StreamingRemote.setConfiguration") {
    CHECK(argc == 1);
    const auto config = json::parse(argv[0]);
    setJsonConfig(config);
    DebugPrint("new configuration: {}", config.dump());
    emit configurationChanged(mConfig);
    return true;
  }

  DebugPrint("No matching function.");

  return false;
}

void XSplit::startOutput(const std::string& id) {
  LOG_FUNCTION(id);
  callJSPlugin("streamingRemoteStartOutput", id);
}

void XSplit::stopOutput(const std::string& id) {
  LOG_FUNCTION(id);
  callJSPlugin("streamingRemoteStopOutput", id);
}

void XSplit::sendToXSplitDebugLog(const std::string& what) {
  callJSPlugin("streamingRemoteDebugLog", what);
}
