#pragma once

#include <cstdlib> // For malloc and free
#include <cstddef> // For std::byte
#include <new>     // For placement new

class storage_allocator {
  public:
    inline explicit storage_allocator(size_t bytes) : m_size(bytes) {
        m_buffer = reinterpret_cast<std::byte *>(malloc(m_size));
        m_offset = m_buffer;
    }

    template <typename T> inline T *alloc() {
        void *offset = m_offset;
        m_offset += sizeof(T);
        return reinterpret_cast<T *>(offset);
    }

    inline storage_allocator(const storage_allocator &) = delete;
    inline storage_allocator &operator=(const storage_allocator &) = delete;

    inline ~storage_allocator() {
        free(m_buffer);
    }

  private:
    size_t m_size;
    std::byte *m_buffer;
    std::byte *m_offset;
};
