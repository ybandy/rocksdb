//  Copyright (c) 2024 Kioxia Corporation.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <stdlib.h>
#include <sys/mman.h>
#include <numaif.h>

#include "util/cxl_memory_allocator.h"


namespace rocksdb {

CXLMemoryAllocator::CXLMemoryAllocator(size_t capacity, size_t block_size, uint64_t nodemask)
    : capacity_(capacity),
      block_size_(block_size) {
  uint8_t* tmp = (uint8_t*)malloc(capacity_);
  for(size_t i = 0; i < capacity_; i++) tmp[i] = i & 0xff;
  pool_ = (char *)mmap(NULL, capacity_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
  if(mbind(pool_, capacity_, MPOL_INTERLEAVE, &nodemask, sizeof(nodemask) * 8, 0)) {
    fprintf(stderr, "can't mbind while constructing CXLMemoryAllocator\n");
    exit(1);
  }
  memcpy(pool_, tmp, capacity_);
  free(tmp);

  for(size_t i = 0; i < capacity_ / block_size_; i++) {
    q.enqueue((void*)(&pool_[block_size_ * i]));
  }
}

CXLMemoryAllocator::~CXLMemoryAllocator() {
  munmap(pool_, capacity_);
}

void* CXLMemoryAllocator::Allocate(size_t size) {
  void* ret = nullptr;
  q.try_dequeue(ret);
  return ret;
}

void CXLMemoryAllocator::Deallocate(void* p) {
  q.enqueue(p);
}

size_t CXLMemoryAllocator::UsableSize(void* p, size_t allocation_size) const {
  return block_size_;
}

} // namespace rocksdb
