/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "XSplit.h"

#include <asio.hpp>
#include <fmt/format.h>

#include <memory>
#include <set>
#include <thread>

#include "Core/Config.h"
#include "Core/Logger.h"
#include "version.h"

#define DebugPrint(...) Logger::debug(__VA_ARGS__)

using json = nlohmann::json;

struct XSplit::Promise {
  public:
    Promise(
      asio::io_context& ctx
    ): p(
      new Impl {
        .timer = asio::steady_timer(ctx, std::chrono::steady_clock::time_point::max()),
      }
    ) {
    }

    void resolve(const nlohmann::json& data) noexcept {
      p->data = data;
      p->timer.cancel();
    }

    asio::awaitable<nlohmann::json> async_wait() {
      asio::error_code ec;
      co_await p->timer.async_wait(asio::redirect_error(asio::use_awaitable, ec));
      assert(ec == asio::error::operation_aborted);
      co_return result();
    }

    nlohmann::json result() const {
      return p->data;
    }
  private:
    struct Impl {
      asio::steady_timer timer;
      nlohmann::json data;
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

asio::awaitable<std::vector<Output>> XSplit::getOutputs() {
  LOG_FUNCTION();
	const auto outputs = co_await coCallJSPlugin("getOutputs");

	std::vector<Output> out;
	for (const auto& output: outputs) {
		out.push_back(Output::fromJson(output));
	}
	co_return out;

}

asio::awaitable<std::vector<Scene>> XSplit::getScenes() {
  LOG_FUNCTION();
  const auto scenes = co_await coCallJSPlugin("getScenes");

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

asio::awaitable<bool> XSplit::activateScene(const std::string& id) {
  LOG_FUNCTION(id);
  co_return (co_await coCallJSPlugin("activateScene", id)).get<bool>();
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

void XSplit::pluginfunc_outputStateChanged(
  const std::string& id,
  const std::string& stateStr) {
  LOG_FUNCTION();
  DebugPrint("State changed: {} => {}", id, stateStr);
  const auto state = Output::stateFromString(stateStr);
  emit outputStateChanged(id, state);
}

void XSplit::pluginfunc_currentSceneChanged(const std::string& id) {
  LOG_FUNCTION();
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

  if (!initialized.wasEmitted()) {
    emit initialized(mConfig);
  } else {
    emit configurationChanged(mConfig);
  }
}

void XSplit::pluginfunc_returnValue(const nlohmann::json& data) {
  LOG_FUNCTION();
  auto key = std::stoull(data["call_id"].get<std::string>());
  mPromises.at(key).resolve(data["value"]);
  mPromises.erase(key);
}
