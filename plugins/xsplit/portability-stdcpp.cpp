/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include "portability.h"

namespace {
// Windows BSTRs have a similar struct, though two size fields: one in chars,
// one in bytes. We just use chars here.
struct BSTRImpl {
  size_t size;
  wchar_t data;
};
}// namespace

BSTR NEW_BSTR_FROM_STDSTRING(const std::string& str) {
  mbstate_t state = mbstate_t();
  const char* src = str.data();
  size_t wlen = 1 + mbsrtowcs(nullptr, &src, 0, &state);
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    malloc(offsetof(BSTRImpl, data) + (wlen * sizeof(wchar_t))));
  impl->size = str.size();
  mbsrtowcs(&impl->data, &src, wlen, &state);
  return &impl->data;
}

std::string STDSTRING_FROM_BSTR(BSTR str) {
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    reinterpret_cast<intptr_t>(str) - offsetof(BSTRImpl, data));
  mbstate_t state = mbstate_t();
  const wchar_t* src = &impl->data;
  size_t len = 1 + wcsrtombs(nullptr, &src, 0, &state);
  std::string out;
  out.reserve(len);
  wcsrtombs(&out.data()[0], &src, len, &state);
  out.resize(len - 1);
  return out;
}
void DELETE_BSTR(BSTR str) {
  BSTRImpl* impl = reinterpret_cast<BSTRImpl*>(
    reinterpret_cast<intptr_t>(str) - offsetof(BSTRImpl, data));
  delete impl;
}
