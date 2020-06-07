/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "portability.h"

#ifdef WIN32
#include <cwchar>

// BSTR is supposed to be an unsigned long length followed by wchar_t[] - but
// XSplit is just giving us wchar_t[] and calling it a BSTR
BSTR NEW_BSTR_FROM_QSTRING(const QString& str) {
  BSTR ret = ::SysAllocStringLen(nullptr, str.size());
  str.toWCharArray(ret);
  ret[str.length()] = 0;
  return ret;
}

QString QSTRING_FROM_BSTR(BSTR str) {
  return QString::fromWCharArray(str, ::SysStringLen(str));
}

void DELETE_BSTR(BSTR str) {
  ::SysFreeString(str);
}
#else

namespace {
// Windows BSTRs have a similar struct, though two size fields: one in chars,
// one in bytes. We just use chars here.
struct BSTRImpl {
  size_t size;
  wchar_t data;
};
}// namespace

BSTR NEW_BSTR_FROM_QSTRING(const QString& str) {
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    malloc(offsetof(BSTRImpl, data) + (str.size() * sizeof(wchar_t))));
  impl->size = str.size();
  str.toWCharArray(&impl->data);
  return &impl->data;
}

QString QSTRING_FROM_BSTR(BSTR str) {
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    reinterpret_cast<intptr_t>(str) - offsetof(BSTRImpl, data));
  return QString::fromWCharArray(str, impl->size);
}
void DELETE_BSTR(BSTR str) {
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    reinterpret_cast<intptr_t>(str) - offsetof(BSTRImpl, data));
  delete impl;
}
#endif
