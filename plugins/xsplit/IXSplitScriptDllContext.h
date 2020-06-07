/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include "portability.h"

#ifdef WIN32
__declspec(uuid("9A554D8A-912F-4F1E-9A26-CC002B3B99BD"))
#endif
  DECLARE_INTERFACE_(IXSplitScriptDllContext, IUnknown) {
  STDMETHOD(Callback)
  (BSTR functionName, BSTR * argumentsArray, UINT argumentsCount) PURE;
};
