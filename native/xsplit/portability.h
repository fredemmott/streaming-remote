/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

/* XSplit only works on Windows, but it's handy to build the plugin
 * on other platforms to make sure you don't break it when
 * developing */
#ifdef _MSC_VER
#include "portability-windows.h"
#else
#include "portability-stdcpp.h"
#endif

#include <string>

BSTR NEW_BSTR_FROM_STDSTRING(const std::string& x);
std::string STDSTRING_FROM_BSTR(BSTR str);
void DELETE_BSTR(BSTR);
