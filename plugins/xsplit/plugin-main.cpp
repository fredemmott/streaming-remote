/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "IXSplitScriptDllContext.h"
#include "XSplit.h"
#include "portability.h"
#include "base/SocketServer.h"

#include <QCoreApplication>
#include <QObject>
#include <QThread>

#ifdef WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
  DWORD  ul_reason_for_call,
  LPVOID lpReserved
){
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
#endif

namespace {
  XSplit* xsplit = nullptr;
  SocketServer* server = nullptr;
  QThread* appThread = nullptr;
  int argc = 0;
  char* argv[1] = { "HelloXSplit" };
}

extern "C" {

  STREAMINGREMOTE_EXPORT
  BOOL WINAPI XSplitScriptPluginInit() {
    appThread = QThread::create(
      [] {
        auto app = new QCoreApplication(argc, argv);
        xsplit = new XSplit(app);
        server = new SocketServer(xsplit);
        QObject::connect(xsplit, &XSplit::initialized, server, &SocketServer::startListening);
        QCoreApplication::exec();
        delete server;
        server = nullptr;
        delete xsplit;
        xsplit = nullptr;
      }
    );
    appThread->start();
    return true;
  }

  STREAMINGREMOTE_EXPORT
  BOOL WINAPI XSplitScriptPluginCall(IXSplitScriptDllContext* pContext, BSTR functionName, BSTR* argv, UINT argc, BSTR * retv) {
    if (xsplit) {
      bool success;
      // Execute in the worker thread, but block on it succeeding
      QMetaObject::invokeMethod(
        xsplit,
        [=, &success] { success = xsplit->handleCall(pContext, functionName, argv, argc, retv); },
        Qt::BlockingQueuedConnection
      );
      return success;
    }
    return false;
  }

  STREAMINGREMOTE_EXPORT
  void WINAPI XSplitScriptPluginDestroy() {
    QCoreApplication::instance()->quit();
    appThread->wait();
    delete appThread;
    appThread = nullptr;
  }
}
