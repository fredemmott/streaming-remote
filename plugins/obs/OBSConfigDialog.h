/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <QDialog>
#include "base/Config.h"

class OBSConfigDialog : public QDialog {
  Q_OBJECT

 public:
  OBSConfigDialog(const Config& config, QWidget* parent = nullptr);

 signals:
  void configChanged(const Config&);

 private:
  Config config;
};
