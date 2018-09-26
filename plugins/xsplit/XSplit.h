/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "IXSplitScriptDllContext.h"
#include "base/Config.h"
#include "base/StreamingSoftware.h"

class QJsonDocument;

class XSplit : public StreamingSoftware {
  Q_OBJECT;

  public:
    XSplit(QObject* parent = nullptr);
    ~XSplit();

    Config getConfiguration() const;
    QList<Output> getOutputs();

    bool handleCall(
      IXSplitScriptDllContext* context,
      BSTR functionName,
      BSTR* argv,
      UINT argc,
      BSTR* retv
    );
  signals:
    void initialized();

  public slots:
    void startOutput(const QString& id);
    void stopOutput(const QString& id);

  private:
    Config config;
    IXSplitScriptDllContext* eventHandlerContext;
    QList<Output> outputs;
    void debugLog(const QString&);
    void setJsonConfig(const QJsonDocument& jsonConfig);
};
