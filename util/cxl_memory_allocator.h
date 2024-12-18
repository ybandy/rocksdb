//  Copyright (c) 2024 Kioxia Corporation.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#pragma once

#include <photon/photon.h>
#include <photon/thread/std-compat.h>
#include "rocksdb/memory_allocator.h"
#include "../../concurrentqueue/concurrentqueue.h"


namespace rocksdb {

class CXLMemoryAllocator : public MemoryAllocator {
 public:
  CXLMemoryAllocator(size_t capacity, size_t block_size, uint64_t nodemask);
  ~CXLMemoryAllocator();

  const char* Name() const override { return "CXLMemoryAllocator"; }
  void* Allocate(size_t size) override;
  void Deallocate(void* p) override;
  size_t UsableSize(void* p, size_t allocation_size) const override;

 private:
  size_t capacity_ = 0;
  size_t block_size_ = 0;
  char *pool_ = nullptr;
  moodycamel::ConcurrentQueue<void*> q;
};

} // namespace rocksdb
