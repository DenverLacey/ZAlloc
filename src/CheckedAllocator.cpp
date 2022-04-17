#include "CheckedAllocator.h"

#include <stdio.h>

#ifdef ZLC_CHECKED_ALLOCATOR_TERMINATE_ON_FREE_ERROR
    #include <exception>
    #define COMPLETE_FAILED_FREE() std::terminate()
#else
    #define COMPLETE_FAILED_FREE() return
#endif

namespace zlc {
    CheckedAllocator::CheckedAllocator(Allocator& inner) :
        inner(inner)
    {
    }

    CheckedAllocator::~CheckedAllocator() {
        this->perform_check();
    }

    void CheckedAllocator::perform_check() {
        if (this->allocations.empty()) return;

        //
        // Not all allocations were freed
        //

        fprintf(stderr, "[ERR] Memory Leaks!\n\tDetected %zu %s that were not freed.\n", this->allocations.size(), this->allocations.size() == 1 ? "allocation" : "allocations");

        size_t i = 0;
        for (uintptr_t p : this->allocations) {
            fprintf(stderr, "\t%zu. %p\n", ++i, p);
        }
    }

    Allocation<uint8_t> CheckedAllocator::on_alloc(size_t count, uint32_t align) {
        auto allocation = this->inner.on_alloc(count, align);
        
        if (allocation.data != nullptr) {
            this->allocations.insert(reinterpret_cast<uintptr_t>(allocation.data));
        }

        return allocation;
    }

    std::optional<size_t> CheckedAllocator::on_resize(Allocation<uint8_t> buf, uint32_t buf_align, size_t new_size) {
        return this->inner.on_resize(buf, buf_align, new_size);
    }

    void CheckedAllocator::on_free(Allocation<uint8_t> buf, uint32_t buf_align) {
        // Check for double frees
        {
            auto it = this->frees.find(reinterpret_cast<uintptr_t>(buf.data));
            if (it != this->frees.end()) {
                fprintf(stderr, 
                    "[ERR] Attempt to free already freed memory!\n"
                    "\tTried to free %zu %s at %p when it has already been freed.\n",
                    buf.count,
                    buf.count == 1 ? "byte" : "bytes",
                    buf.data
                );
                COMPLETE_FAILED_FREE();
            }
        }

        auto it = this->allocations.find(reinterpret_cast<uintptr_t>(buf.data));
        if (it == this->allocations.end()) {
            fprintf(stderr, 
                "[ERR] Attempt to free memory with wrong allocator or unallocated memory!\n"
                "\tTried to free %zu %s at %p with an allocator that does not own that memory.\n", 
                buf.count,
                buf.count == 1 ? "byte" : "bytes",
                buf.data
            );
            COMPLETE_FAILED_FREE();
        }

        this->allocations.erase(it);
        this->frees.insert(*it);

        this->inner.on_free(buf, buf_align);
    }
}

#undef COMPLETE_FAILED_FREE
