//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#pragma once

#include <functional>
#include <string>

#include "port/port.h"
#include "rocksdb/env.h"
#include "util/mock_time_env.h"
#include "util/mutexlock.h"

namespace rocksdb {

class RepeatableThread {
 public:
  RepeatableThread(std::function<void()> function,
                   const std::string& thread_name, Env* env, uint64_t delay_us,
                   uint64_t initial_delay_us = 0)
      : function_(function),
        thread_name_("rocksdb:" + thread_name),
        env_(env),
        delay_us_(delay_us),
        initial_delay_us_(initial_delay_us),
        mutex_(env),
        cond_var_(&mutex_),
        running_(true),
#ifndef NDEBUG
        waiting_(false),
        run_count_(0),
#endif
        thread_([this] { thread(); }) {
  }

  void cancel() {
    {
      InstrumentedMutexLock l(&mutex_);
      if (!running_) {
        return;
      }
      running_ = false;
      cond_var_.SignalAll();
    }
    thread_.join();
  }

  bool IsRunning() { return running_; }

  ~RepeatableThread() { cancel(); }

#ifndef NDEBUG
  // Wait until RepeatableThread starting waiting, call the optional callback,
  // then wait for one run of RepeatableThread. Tests can use provide a
  // custom env object to mock time, and use the callback here to bump current
  // time and trigger RepeatableThread. See repeatable_thread_test for example.
  //
  // Note: only support one caller of this method.
  void TEST_WaitForRun(std::function<void()> callback = nullptr) {
    InstrumentedMutexLock l(&mutex_);
    while (!waiting_) {
      cond_var_.Wait();
    }
    uint64_t prev_count = run_count_;
    if (callback != nullptr) {
      callback();
    }
    cond_var_.SignalAll();
    while (!(run_count_ > prev_count)) {
      cond_var_.Wait();
    }
  }
#endif

 private:
  bool wait(uint64_t delay) {
    InstrumentedMutexLock l(&mutex_);
    if (running_ && delay > 0) {
      uint64_t wait_until = env_->NowMicros() + delay;
#ifndef NDEBUG
      waiting_ = true;
      cond_var_.SignalAll();
#endif
      while (running_) {
        cond_var_.TimedWait(wait_until);
        if (env_->NowMicros() >= wait_until) {
          break;
        }
      }
#ifndef NDEBUG
      waiting_ = false;
#endif
    }
    return running_;
  }

  void thread() {
    assert(delay_us_ > 0);
    if (!wait(initial_delay_us_)) {
      return;
    }
    do {
      function_();
#ifndef NDEBUG
      {
        InstrumentedMutexLock l(&mutex_);
        run_count_++;
        cond_var_.SignalAll();
      }
#endif
    } while (wait(delay_us_));
  }

  const std::function<void()> function_;
  const std::string thread_name_;
  Env* const env_;
  const uint64_t delay_us_;
  const uint64_t initial_delay_us_;

  // Mutex lock should be held when accessing running_, waiting_
  // and run_count_.
  InstrumentedMutex mutex_;
  InstrumentedCondVar cond_var_;
  bool running_;
#ifndef NDEBUG
  // RepeatableThread waiting for timeout.
  bool waiting_;
  // Times function_ had run.
  uint64_t run_count_;
#endif
  port::Thread thread_;
};

}  // namespace rocksdb
