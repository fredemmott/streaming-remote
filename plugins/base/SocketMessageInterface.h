/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "MessageInterface.h"

class QIODevice;

class SocketMessageInterface : public MessageInterface {
  Q_OBJECT

 public:
  explicit SocketMessageInterface(QIODevice* socket, QObject* parent = nullptr);
 public slots:
  void sendMessage(const QByteArray& message);
 private slots:
  void readyRead();

 private:
  QIODevice* socket;
};
