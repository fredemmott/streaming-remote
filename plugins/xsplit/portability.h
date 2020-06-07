/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <QString>

/* XSplit only works on Windows, but it's handy to build the plugin
 * on other platforms to make sure you don't break it when
 * developing */
#ifdef WIN32

#include <windows.h>
#define STREAMINGREMOTE_EXPORT __declspec(dllexport)

#else// WIN32

typedef wchar_t* BSTR;
typedef unsigned int UINT;
typedef bool BOOL;
#define DECLARE_INTERFACE_(NAME, COMBASE_IGNORED) class NAME
#define STDMETHOD(NAME) \
 public: \
  virtual void NAME
#define PURE = 0
#define WINAPI
#include <cwchar>

#define STREAMINGREMOTE_EXPORT

#endif

BSTR NEW_BSTR_FROM_QSTRING(const QString& x);
QString QSTRING_FROM_BSTR(BSTR str);
void DELETE_BSTR(BSTR);
