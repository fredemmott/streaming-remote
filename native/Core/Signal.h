/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <map>

namespace {
template <typename...>
struct is_empty : std::true_type {};
template <typename Tfirst, typename... Trest>
struct is_empty<Tfirst, Trest...> : std::false_type {};

}// namespace

class ConnectionImplBase {
  public:
    virtual void disconnect() = 0;
    virtual ~ConnectionImplBase();
};

typedef std::unique_ptr<ConnectionImplBase> Connection;

class ScopedConnection {
  public:
    ScopedConnection(Connection);
    ScopedConnection(ScopedConnection&& other) = default;
    ~ScopedConnection();
  private:
    Connection mConnection;
};

template <typename... Targs>
class Signal {
 public:
  typedef std::function<void(Targs...)> Callback;
  Signal() {
  }
  Signal(const Signal<Targs...>& other) = delete;

  void operator()(Targs... args) {
    mEmitted = true;
    for (auto& [id, callback] : mCallbacks) {
      callback(args...);
    }
  }

  Connection connect(const Callback& callback) {
    const auto key = mNextKey++;
    mCallbacks.emplace(key, callback);
    return std::make_unique<ConnectionImpl>(this, key);
  }

  template <typename TClass, typename TRet>
  Connection connect(TClass* instance, TRet (TClass::*method)(Targs...)) {
    return connect([=](Targs... args) { (instance->*method)(args...); });
  }

  template <
    typename TClass,
    typename TRet,
    // disable if Targs is empty, otherwise this conflicts with the above
    // definition
    typename = std::enable_if<!is_empty<Targs...>::value>>
  Connection connect(TClass* instance, TRet (TClass::*method)()) {
    return connect([=](Targs... args) { (instance->*method)(); });
  }

  bool wasEmitted() {
    return mEmitted;
  }

 private:
  uint64_t mNextKey = 0;
  std::map<uint64_t, Callback> mCallbacks;
  bool mEmitted = false;

  class ConnectionImpl final : public ConnectionImplBase {
    public:
      ConnectionImpl(
        Signal<Targs...>* signal,
        uint64_t key
      ): mSignal(signal), mKey(key) {}

      virtual void disconnect() override {
        mSignal->mCallbacks.erase(mKey);
      }

      ~ConnectionImpl() {
      }
    private:
      Signal<Targs...>* mSignal;
      uint64_t mKey;
  };
};

#define emit /* emit */

class ConnectionOwner {
  public:
    ~ConnectionOwner();
  protected:
    template<typename... Targs>
    void connect(Signal<Targs...>& signal, typename Signal<Targs...>::Callback callback) {
      mConnections.push_back(std::move(ScopedConnection(std::move(signal.connect(callback)))));
    }

    template <typename TClass, typename TRet, typename ...Targs>
    void connect(Signal<Targs...>& signal, TClass* instance, TRet (TClass::*method)(Targs...)) {
      connect(signal, [=](Targs... args) { (instance->*method)(args...); });
    }
  private:
    std::vector<ScopedConnection> mConnections;
};
