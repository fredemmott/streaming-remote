/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "OBS.h"
#include "OBSConfigDialog.h"

#include <util/config-file.h>

#include <QAction>
#include <QMainWindow>
#include <QObject>

namespace {
const std::string s_recording("Recording");
const std::string s_streaming("Streaming");

const char* CONFIG_SECTION = "streamingRemote";
const char* CONFIG_ID_PASSWORD = "password";
const char* CONFIG_ID_TCP_PORT = "tcpPort";
const char* CONFIG_ID_WEBSOCKET_PORT = "webSocketPort";

struct OBSSourceListScopeGuard {
  obs_frontend_source_list* sources = nullptr;
  ~OBSSourceListScopeGuard() {
    if (sources) {
      obs_frontend_source_list_free(sources);
    }
    sources = nullptr;
  }
};

}// namespace

OBS::OBS() : QObject(), StreamingSoftware(), mLoggerImpl(
  [=](const std::string& message) {
    blog(LOG_INFO, "[obs-streaming-remote] %s", message.c_str());
  }
) {
  LOG_FUNCTION();
  obs_frontend_add_event_callback(&OBS::frontendEventCallback, this);
  mConfig = getInitialConfiguration();

  auto obsWindow
    = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
  if (!obsWindow) {
    return;
  }

  auto settingsAction = reinterpret_cast<QAction*>(
    obs_frontend_add_tools_menu_qaction("Streaming Remote"));

  connect(settingsAction, &QAction::triggered, [this, obsWindow]() {
    auto dialog = new OBSConfigDialog(mConfig, obsWindow);
    connect(
      dialog, &OBSConfigDialog::configChanged, this, &OBS::setConfiguration);
    dialog->show();
  });

  emit initialized(mConfig);
}

OBS::~OBS() {
  LOG_FUNCTION();
  obs_frontend_remove_event_callback(&OBS::frontendEventCallback, this);
}

std::vector<Output> OBS::getOutputs() {
  LOG_FUNCTION();
  auto profile = obs_frontend_get_profile_config();
  const bool delayEnabled = config_get_bool(profile, "Output", "DelayEnable");
  const int delaySeconds
    = delayEnabled ? config_get_int(profile, "Output", "DelaySec") : 0;
  return std::vector<Output>{
    Output{
      s_recording, s_recording,
      obs_frontend_recording_active() ? OutputState::ACTIVE
                                      : OutputState::STOPPED,
      OutputType::LOCAL_RECORDING,
      -1// delays not supported
    },
    Output{s_streaming, s_streaming,
           obs_frontend_streaming_active() ? OutputState::ACTIVE
                                           : OutputState::STOPPED,
           OutputType::REMOTE_STREAM, delaySeconds}};
}

std::vector<Scene> OBS::getScenes() {
  LOG_FUNCTION();

  const auto active_scene = obs_frontend_get_current_scene();

  obs_frontend_source_list sources = {};
  OBSSourceListScopeGuard guard = {&sources};

  obs_frontend_get_scenes(&sources);
  std::vector<Scene> out;
  for (size_t i = 0; i < sources.sources.num; i++) {
    const auto source = sources.sources.array[i];
    // OBS sources also have an 'id' property, but it always 'scene' for
    // every scene, so not useful as a unique identifier
    const auto name = obs_source_get_name(source);
    out.push_back({
      /*.id = */ name,
      /*.name = */ name,
      /*.active = */ (source == active_scene)
    });
  }
  return out;
}

bool OBS::activateScene(const std::string& id) {
  LOG_FUNCTION();
  Logger::debug("Activating scene '{}'", id);
  const auto current_scene = obs_frontend_get_current_scene();

  obs_frontend_source_list sources = {};
  OBSSourceListScopeGuard guard = {&sources};

  obs_frontend_get_scenes(&sources);

  for (size_t i = 0; i < sources.sources.num; i++) {
    const auto source = sources.sources.array[i];
    if (source == current_scene) {
      continue;
    }
    // OBS sources also have an 'id' property, but it always 'scene' for
    // every scene, so not useful as a unique identifier
    const auto name = obs_source_get_name(source);
    if (name == id) {
      obs_frontend_set_current_scene(source);
      return true;
    }
  }
  return false;
}

void OBS::startOutput(const std::string& id) {
  LOG_FUNCTION();
  Logger::debug("Starting output '{}'", id);
  if (id == s_recording) {
    obs_frontend_recording_start();
    return;
  }
  if (id == s_streaming) {
    obs_frontend_streaming_start();
    return;
  }
}

void OBS::stopOutput(const std::string& id) {
  LOG_FUNCTION();
  Logger::debug("Starting output '{}'", id);
  if (id == s_recording) {
    obs_frontend_recording_stop();
    return;
  }
  if (id == s_streaming) {
    obs_frontend_streaming_stop();
    // Clients may depend on the state change to indicate that stopping did
    // anything; emit the event if there's a delay.
    //
    // OBS fix: https://github.com/obsproject/obs-studio/pull/1518
    if (obs_output_get_active_delay(obs_frontend_get_streaming_output()) > 0) {
      emit this->outputStateChanged(s_streaming, OutputState::STOPPING);
    }
    return;
  }
}

Config OBS::getConfiguration() const {
  LOG_FUNCTION();
  return mConfig;
}

bool OBS::setOutputDelay(const std::string& id, int64_t seconds) {
  LOG_FUNCTION();
  if (id != s_streaming) {
    return false;
  }
  auto config = obs_frontend_get_profile_config();
  config_set_bool(config, "Output", "DelayEnable", seconds > 0);
  if (seconds <= 0) {
    return true;
  }
  config_set_int(config, "Output", "DelaySec", seconds);
  return true;
}

void OBS::setConfiguration(const Config& config) {
  LOG_FUNCTION();
  auto obs_config = obs_frontend_get_global_config();
  assert(obs_config);
  if (!obs_config) {
    return;
  }
  mConfig = config;
  config_set_string(
    obs_config, CONFIG_SECTION, CONFIG_ID_PASSWORD, config.password.c_str());
  config_set_uint(
    obs_config, CONFIG_SECTION, CONFIG_ID_TCP_PORT, config.tcpPort);
  config_set_uint(
    obs_config, CONFIG_SECTION, CONFIG_ID_WEBSOCKET_PORT, config.webSocketPort);
  config_save(obs_config);

  emit configurationChanged(config);
}

Config OBS::getInitialConfiguration() {
  LOG_FUNCTION();
  Config config = Config::getDefault();

  config_t* obs_config = obs_frontend_get_global_config();
  if (!obs_config) {
    return config;
  }

  const auto password
    = config_get_string(obs_config, CONFIG_SECTION, CONFIG_ID_PASSWORD);
  if (password) {
    config.password = password;
  } else {
    config_set_string(
      obs_config, CONFIG_SECTION, CONFIG_ID_PASSWORD, config.password.c_str());
  }

  config_set_default_uint(
    obs_config, CONFIG_SECTION, CONFIG_ID_TCP_PORT, config.tcpPort);
  config.tcpPort
    = config_get_uint(obs_config, CONFIG_SECTION, CONFIG_ID_TCP_PORT);

  config_set_default_uint(
    obs_config, CONFIG_SECTION, CONFIG_ID_WEBSOCKET_PORT, config.webSocketPort);
  config.webSocketPort
    = config_get_uint(obs_config, CONFIG_SECTION, CONFIG_ID_WEBSOCKET_PORT);
  return config;
}

void OBS::frontendEventCallback(enum obs_frontend_event event, void* data) {
  OBS* obs = reinterpret_cast<OBS*>(data);
  switch (event) {
    case OBS_FRONTEND_EVENT_STREAMING_STARTING:
      emit obs->outputStateChanged(s_streaming, OutputState::STARTING);
      break;
    case OBS_FRONTEND_EVENT_STREAMING_STARTED:
      emit obs->outputStateChanged(s_streaming, OutputState::ACTIVE);
      break;
    case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
      emit obs->outputStateChanged(s_streaming, OutputState::STOPPING);
      break;
    case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
      emit obs->outputStateChanged(s_streaming, OutputState::STOPPED);
      break;
    case OBS_FRONTEND_EVENT_RECORDING_STARTING:
      emit obs->outputStateChanged(s_recording, OutputState::STARTING);
      break;
    case OBS_FRONTEND_EVENT_RECORDING_STARTED:
      emit obs->outputStateChanged(s_recording, OutputState::ACTIVE);
      break;
    case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
      emit obs->outputStateChanged(s_recording, OutputState::STOPPING);
      break;
    case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
      emit obs->outputStateChanged(s_recording, OutputState::STOPPED);
    case OBS_FRONTEND_EVENT_SCENE_CHANGED:
      // 'id' for scenes is always 'scene', so use name instead
      emit obs->currentSceneChanged(
        obs_source_get_name(obs_frontend_get_current_scene())
      );
      break;
  }
}
