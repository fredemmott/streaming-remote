/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <cwchar>
#include <memory>

typedef wchar_t* BSTR;
typedef unsigned int UINT;
typedef bool BOOL;
template <class T>
struct CComPtr {
  CComPtr(T* ptr) : p(ptr) {
  }
  T& operator*() {
    return *p;
  }
  T* operator->() {
    return p;
  }
  void operator=(T* other) {
    p = other;
  }
  operator T*() {
    return p;
  }
  T* p;
};
#define interface class
#define DECLSPEC_UUID(uuid)
#define STDMETHOD(NAME) \
 public: \
  virtual void NAME
#define PURE = 0
#define WINAPI

#define STREAMINGREMOTE_EXPORT

class IUnknown {};
