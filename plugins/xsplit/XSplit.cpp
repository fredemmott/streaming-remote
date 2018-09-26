/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "XSplit.h"

#include "base/Config.h"

#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>

#define CHECK(x) \
  if (!(x)) { \
    this->debugLog(QString("Assertion failed at %1:%2: %3").arg(__FILE__).arg(__LINE__).arg(#x)); \
    return false; \
  }

XSplit::XSplit(QObject* parent) : StreamingSoftware(parent),
eventHandlerContext(nullptr) {
}

XSplit::~XSplit() {
}

Config XSplit::getConfiguration() const {
  return config;
}

QList<Output> XSplit::getOutputs() {
  return this->outputs;
}

void XSplit::setJsonConfig(const QJsonDocument& doc) {
  {
    const auto v = doc["password"];
    if (v.isNull() || (v.isString() && v == "")) {
      config.password = QString();
    } else {
      config.password = v.toString();
    }
  }
  {
    const auto v = doc["localSocket"];
    if (v.isNull() || (v.isString() && v == "")) {
      config.localSocket = QString();
    } else {
      config.localSocket = v.toString();
    }
  }
  config.tcpPort = doc["tcpPort"].toInt();
  config.webSocketPort = doc["webSocketPort"].toInt();
}

bool XSplit::handleCall(IXSplitScriptDllContext* context, BSTR functionName, BSTR* argv, UINT argc, BSTR* retv) {
  if (wcscmp(functionName, L"StreamingRemote.init") == 0) {
    CHECK(argc == 2);
    this->eventHandlerContext = context;
    QJsonDocument configJson  = QJsonDocument::fromJson(QSTRING_FROM_BSTR(argv[0]).toUtf8());
    QJsonDocument outputsJson = QJsonDocument::fromJson(QSTRING_FROM_BSTR(argv[1]).toUtf8());

    setJsonConfig(configJson);

    outputs.clear();
    for (const auto& output : outputsJson.array()) {
      outputs.push_back(Output::fromJson(output.toObject()));
    }
    emit initialized();
    this->debugLog("Initialized");
    return true;
  }

  if (wcscmp(functionName, L"StreamingRemote.outputStateChanged") == 0) {
    CHECK(argc == 2);
    const QString id(QSTRING_FROM_BSTR(argv[0]));
    const QString stateStr = QSTRING_FROM_BSTR(argv[1]);
    const OutputState state(Output::stateFromString(stateStr));
    this->debugLog(QString("State changed: %1 => %2 (%3)").arg(id).arg(stateStr).arg(Output::stateToString(state)));
    for (auto& output: this->outputs) {
      if (output.id == id) {
        output.state = state;
        break;
      }
    }
    emit outputStateChanged(id, state);
    return true;
  }

  if (wcscmp(functionName, L"StreamingRemote.getDefaultConfiguration") == 0) {
    CHECK(argc == 0);
    auto config = Config::getDefault();
    QJsonDocument doc(QJsonObject {
      { "password", config.password },
      { "localSocket", config.localSocket },
      { "tcpPort", config.tcpPort },
      { "webSocketPort", config.webSocketPort }
    });
    *retv = NEW_BSTR_FROM_QSTRING(QString::fromUtf8(doc.toJson()));
    return true;
  }

  if (wcscmp(functionName, L"StreamingRemote.setConfiguration") == 0) {
    CHECK(argc == 1);
    const QString json(QSTRING_FROM_BSTR(argv[0]));
    const auto doc = QJsonDocument::fromJson(json.toUtf8());
    setJsonConfig(doc);
    this->debugLog("Set new configuration");
    emit configurationChanged(config);
    return true;
  }

  return false;
}

void XSplit::startOutput(const QString& id) {
  this->debugLog("starting output: " + id);
  BSTR id_bstr = NEW_BSTR_FROM_QSTRING(id);
  BSTR params[1] = { id_bstr };
  this->eventHandlerContext->Callback(L"streamingRemoteStartOutput", params, 1);
  DELETE_BSTR(id_bstr);
  this->debugLog("stopped output: " + id);
}

void XSplit::stopOutput(const QString& id) {
  this->debugLog("stopping output: " + id);
  BSTR id_bstr = NEW_BSTR_FROM_QSTRING(id);
  BSTR params[1] = { id_bstr };
  this->eventHandlerContext->Callback(L"streamingRemoteStopOutput", params, 1);
  DELETE_BSTR(id_bstr);
  this->debugLog("stopped output: " + id);
}

void XSplit::debugLog(const QString& what) {
  BSTR what_bstr = NEW_BSTR_FROM_QSTRING(what);
  BSTR params[1] = { what_bstr };
  this->eventHandlerContext->Callback(L"streamingRemoteDebugLog", params, 1);
  DELETE_BSTR(what_bstr);
}
