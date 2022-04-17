#include <iostream>
#include <algorithm>
#include <stdlib.h>

#include "Allocator.h"
#include "CheckedAllocator.h"

class Mallocator : public zlc::Allocator {
public:
    zlc::Allocation<uint8_t> on_alloc(size_t size, uint32_t align) override {
        void* data = malloc(size);
        return { size, reinterpret_cast<uint8_t*>(data) };
    }

    std::optional<size_t> on_resize(zlc::Allocation<uint8_t> buf, uint32_t buf_align, size_t new_size) override {
        assert(false);
    }

    void on_free(zlc::Allocation<uint8_t> buf, uint32_t buf_align) override {
        std::free(buf.data);
    }
};

struct Slice {
    size_t size;
    int *items;
};

Slice map(zlc::Allocator& allocator, Slice s, int(*f)(int)) {
    Slice mapped;
    mapped.size = s.size;
    mapped.items = allocator.alloc<int>(s.size).data;

    for (size_t i = 0; i < s.size; i++) {
        mapped.items[i] = f(s.items[i]);
    }

    return mapped;
}

int main() {
    auto mallocator = Mallocator{};
    auto allocator = zlc::CheckedAllocator{ mallocator };

    auto ns = allocator.alloc<int>(10);
    std::fill(ns.data, ns.data + ns.count, 9);

    for (int i = 0; i < 10; i++) {
        printf("[%d]: %d\n", i, ns.data[i]);
    }

    auto doubles = map(allocator, { ns.count, ns.data }, [](int n) { return n * 2; });
    for (size_t i = 0; i < doubles.size; i++) {
        printf("[%zu]: %d\n", i, doubles.items[i]);
    }

    auto n = allocator.create<int>();
    *n = 5;
    printf("n=%d\n", *n);

    allocator.free(ns);
    allocator.free(zlc::Allocation<int>{ doubles.size, doubles.items });
    allocator.destroy(n);
    return 0;
}
