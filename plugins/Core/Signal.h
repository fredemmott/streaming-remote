/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <vector>

namespace {
template <typename...>
struct is_empty : std::true_type {};
template <typename Tfirst, typename... Trest>
struct is_empty<Tfirst, Trest...> : std::false_type {};
}// namespace

template <typename... Targs>
class Signal {
 public:
  typedef std::function<void(Targs...)> Callback;
  Signal() {
  }
  Signal(const Signal<Targs...>& other) = delete;

  void operator()(Targs... args) {
    mEmitted = true;
    for (auto& callback : mCallbacks) {
      callback(args...);
    }
  }

  void connect(const Callback& callback) {
    mCallbacks.push_back(callback);
  }

  template <typename TClass, typename TRet>
  void connect(TClass* instance, TRet (TClass::*method)(Targs...)) {
    connect([=](Targs... args) { (instance->*method)(args...); });
  }

  template <
    typename TClass,
    typename TRet,
    // disable if Targs is empty, otherwise this conflicts with the above
    // definition
    typename = std::enable_if<!is_empty<Targs...>::value>>
  void connect(TClass* instance, TRet (TClass::*method)()) {
    connect([=](Targs... args) { (instance->*method)(); });
  }

  bool wasEmitted() {
    return mEmitted;
  }

 private:
  std::vector<Callback> mCallbacks;
  bool mEmitted = false;
};

#define emit /* emit */
