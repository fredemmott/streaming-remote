/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include <comutil.h>
#include <cwchar>

#include "portability.h"

// BSTR is supposed to be an unsigned long length followed by wchar_t[] - but
// XSplit is just giving us wchar_t[] and calling it a BSTR
BSTR NEW_BSTR_FROM_STDSTRING(const std::string& str) {
  return _bstr_t(str.c_str()).Detach();
}

std::string STDSTRING_FROM_BSTR(BSTR str) {
  return std::string((const char*)_bstr_t(str));
}

void DELETE_BSTR(BSTR str) {
  ::SysFreeString(str);
}
