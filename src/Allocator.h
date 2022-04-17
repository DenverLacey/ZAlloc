#pragma once

#include <exception>
#include <optional>
#include <stddef.h>

namespace zlc {
    template<typename T>
    struct Allocation {
        size_t count;
        T* data;
    };

    class Allocator {
    public:
        virtual ~Allocator() {}

        virtual Allocation<uint8_t> on_alloc(size_t size, uint32_t align) = 0;
        virtual std::optional<size_t> on_resize(Allocation<uint8_t> buf, uint32_t buf_align, size_t new_size) = 0;
        virtual void on_free(Allocation<uint8_t> buf, uint32_t buf_align) = 0;

        template<typename T>
        Allocation<T> alloc(size_t count, uint32_t align = 0) {
            uint32_t _align = align == 0 ? alignof(T) : align;
            auto raw_allocation = this->on_alloc(count * sizeof(T), _align);
            return Allocation<T> {
                raw_allocation.count / sizeof(T),
                reinterpret_cast<T*>(raw_allocation.data)
            };
        }

        template<typename T>
        std::optional<size_t> resize(Allocation<T> buf, size_t new_count, uint32_t align = 0) {
            uint32_t _align = align == 0 ? alignof(T) : align;
            auto byte_buf = Allocation<uint8_t> {
                buf.count * sizeof(T),
                reinterpret_cast<uint8_t*>(buf.data)
            };
            return this->on_resize(byte_buf, _align, new_count * sizeof(T));
        }

        template<typename T>
        void free(Allocation<T> buf, uint32_t buf_align = 0) {
            uint32_t _align = buf_align == 0 ? alignof(T) : buf_align;
            auto byte_buf = Allocation<uint8_t> {
                buf.count * sizeof(T),
                reinterpret_cast<uint8_t*>(buf.data)
            };
            this->on_free(byte_buf, _align);
        }

        template<typename T>
        inline T* create(uint32_t align = 0) {
            uint32_t _align = align == 0 ? alignof(T) : align;
            auto allocation = this->on_alloc(sizeof(T), _align);
            return reinterpret_cast<T*>(allocation.data);
        }

        template<typename T>
        inline void destroy(T* ptr) {
            this->free(Allocation<T>{ 1, ptr });
        }
    };
}
