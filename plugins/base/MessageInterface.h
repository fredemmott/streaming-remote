/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <QByteArray>
#include <QObject>

class MessageInterface : public QObject {
  Q_OBJECT

 protected:
  MessageInterface(QObject* parent = nullptr);
 public slots:
  virtual void sendMessage(const QByteArray& message) = 0;
 signals:
  void messageReceived(const QByteArray& message);
};
