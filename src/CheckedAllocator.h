#pragma once

#include "Allocator.h"

#include <unordered_set>

namespace zlc {
    class CheckedAllocator : public Allocator {
        Allocator& inner;
        std::unordered_set<uintptr_t> allocations;
        std::unordered_set<uintptr_t> frees;

    public:
        CheckedAllocator(Allocator& inner);
        ~CheckedAllocator();

    public:
        Allocation<uint8_t> on_alloc(size_t count, uint32_t align) override;
        std::optional<size_t> on_resize(Allocation<uint8_t> buf, uint32_t buf_align, size_t new_size) override;
        void on_free(Allocation<uint8_t> buf, uint32_t buf_align) override;

    public:
        void perform_check();
    };
}
