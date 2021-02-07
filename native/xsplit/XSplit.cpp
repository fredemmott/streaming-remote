/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "XSplit.h"

#include <fmt/format.h>

#include <memory>
#include <set>
#include <thread>

#include "Core/Config.h"
#include "Core/Logger.h"
#include "version.h"

#define DebugPrint(...) Logger::debug(__VA_ARGS__)

using json = nlohmann::json;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct XSplit::Promise {
  public:
    Promise(): p(new Impl { CreateEventA(NULL, true, false, nullptr) }) {}

    void resolve(const nlohmann::json& data) noexcept {
      p->data = data;
      SetEvent(p->event);
    }

    HANDLE getEvent() const noexcept {
      return p->event;
    }

    nlohmann::json result() const {
      return p->data;
    }
  private:
    struct Impl {
      HANDLE event;
      nlohmann::json data;

      ~Impl() {
        CloseHandle(event);
      }
    };
    std::shared_ptr<Impl> p;
};

#define XSPLIT_CHECK(x) \
  if (!(x)) { \
    DebugPrint("Assertion failed at {}:{}: {}" __FILE__, __LINE__, #x); \
    return false; \
  }

XSplit::XSplit(std::shared_ptr<asio::io_context> io_context, IXSplitScriptDllContext* context)
  : StreamingSoftware(io_context),
    mCallbackImpl(context),
    mLoggerImpl([this](const std::string& message) {
      this->sendToXSplitDebugLog(message);
    }) {
  LOG_FUNCTION();
  registerPluginFunc("init", &XSplit::pluginfunc_init);
  registerPluginFunc("setConfig", &XSplit::pluginfunc_setConfig);
  registerPluginFunc(
    "outputStateChanged", &XSplit::pluginfunc_outputStateChanged);
  registerPluginFunc(
    "getDefaultConfiguration", &XSplit::pluginfunc_getDefaultConfiguration);
  registerPluginFunc("setConfiguration", &XSplit::pluginfunc_setConfiguration);
  registerPluginFunc("currentSceneChanged", &XSplit::pluginfunc_currentSceneChanged);
  registerPluginFunc("returnValue", &XSplit::pluginfunc_returnValue);
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

asio::awaitable<std::vector<Scene>> XSplit::getScenes() {
  LOG_FUNCTION();
  Promise promise;
  auto id = mNextPromiseId++;
  mPromises.emplace(id, promise);
  asio::windows::object_handle obj(getIoContext(), promise.getEvent());
  callJSPlugin("getScenes", std::to_string(id));
  co_await obj.async_wait(asio::use_awaitable);
  const auto scenes = promise.result();
  mPromises.erase(id);
  Logger::debug("Retrieved scenes from promise: {}", scenes.dump());

  std::vector<Scene> out;
  for (const auto& scene : scenes) {
    out.push_back(Scene::fromJson(scene));
  }
  co_return out;
}

void XSplit::setJsonConfig(const nlohmann::json& doc) {
  LOG_FUNCTION();
  mConfig.password = doc["password"].is_null() ? "" : doc["password"];
  mConfig.tcpPort = doc["tcpPort"];
  mConfig.webSocketPort = doc["webSocketPort"];
}

bool XSplit::handleCall(
  IXSplitScriptDllContext* context,
  BSTR wideFunctionName,
  BSTR* argv,
  UINT argc,
  BSTR* ret) {
  LOG_FUNCTION();
  if (context != mCallbackImpl) {
    mCallbackImpl = context;
  }

  const auto fun = STDSTRING_FROM_BSTR(wideFunctionName);

  DebugPrint("Call: {}", fun);
  for (UINT i = 0; i < argc; ++i) {
    DebugPrint("- argv[{}]: {}", i, STDSTRING_FROM_BSTR(argv[i]));
  }

  auto it = mPluginFuncs.find(fun);
  if (it != mPluginFuncs.end()) {
    it->second(ret, argc, argv);
    return true;
  }

  DebugPrint("No matching function.");
  return false;
}

void XSplit::startOutput(const std::string& id) {
  LOG_FUNCTION(id);
  callJSPlugin("startOutput", id);
}

void XSplit::stopOutput(const std::string& id) {
  LOG_FUNCTION(id);
  callJSPlugin("stopOutput", id);
}

bool XSplit::activateScene(const std::string& id) {
  LOG_FUNCTION(id);
  for (const auto& scene: mScenes) {
    if (scene.active) {
      continue;
    }
    if (scene.id == id) {
      callJSPlugin("activateScene", id);
      return true;
    }
  }
  return false;
}

void XSplit::sendToXSplitDebugLog(const std::string& what) {
  callJSPlugin("debugLog", what);
}

void XSplit::pluginfunc_init(const std::string& proto_version) {
  LOG_FUNCTION();
  if (proto_version != XSPLIT_PLUGIN_DLL_API_VERSION) {
    DebugPrint(
      "Protocol version mismatch - JS v{}, DLL v{}", proto_version,
      XSPLIT_PLUGIN_DLL_API_VERSION);
    // intentionally not leaving early - tell the JS plugin what's going on.
  }
  callJSPlugin("init", XSPLIT_PLUGIN_DLL_API_VERSION);
}

void XSplit::pluginfunc_setConfig(
  const nlohmann::json& config,
  const nlohmann::json& outputs,
  const nlohmann::json& scenes) {
  LOG_FUNCTION();
  DebugPrint("Setting config to {}", config.dump());
  setJsonConfig(config);

  DebugPrint("Setting outputs to {}", outputs.dump());
  mOutputs.clear();
  for (const auto& output : outputs) {
    mOutputs.push_back(Output::fromJson(output));
  }
  mScenes.clear();
  DebugPrint("Setting scenes to {}", scenes.dump());
  for (const auto& scene : scenes) {
    mScenes.push_back(Scene::fromJson(scene));
  }
  emit initialized(mConfig);
}

void XSplit::pluginfunc_outputStateChanged(
  const std::string& id,
  const std::string& stateStr) {
  LOG_FUNCTION();
  DebugPrint("State changed: {} => {}", id, stateStr);

  const auto state = Output::stateFromString(stateStr);
  for (auto& output : mOutputs) {
    if (output.id == id) {
      output.state = state;
      break;
    }
  }
  emit outputStateChanged(id, state);
}

void XSplit::pluginfunc_currentSceneChanged(const std::string& id) {
  LOG_FUNCTION();
  for (auto& scene : mScenes) {
    scene.active = (scene.id == id);
  }
  emit currentSceneChanged(id);
}

nlohmann::json XSplit::pluginfunc_getDefaultConfiguration() {
  LOG_FUNCTION();
  auto config = Config::getDefault();
  json doc({{"password", config.password},
            {"tcpPort", config.tcpPort},
            {"webSocketPort", config.webSocketPort}});
  DebugPrint("Returning default config: {}", doc.dump());
  return doc;
}

void XSplit::pluginfunc_setConfiguration(const nlohmann::json& config) {
  LOG_FUNCTION();
  DebugPrint("new configuration: {}", config.dump());
  setJsonConfig(config);
  emit configurationChanged(mConfig);
}

void XSplit::pluginfunc_returnValue(const nlohmann::json& data) {
  LOG_FUNCTION();
  mPromises[std::stoull(data["call_id"].get<std::string>())].resolve(data["value"]);
}
