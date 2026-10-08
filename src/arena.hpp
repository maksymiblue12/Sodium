#pragma once

#include <cstddef>
#include <memory>
#include <utility>

class ArenaAllocator {
public:
	explicit ArenaAllocator(const size_t max_num_bytes)
	:m_size(max_num_bytes),
	m_buffer(new std::byte[max_num_bytes]),
	m_offset(m_buffer) {
	}

	ArenaAllocator(const ArenaAllocator&) = delete;
	ArenaAllocator& operator=(const ArenaAllocator&) = delete;

	template <typename T>
	[[nodiscard]] T* alloc() {
		size_t remaining_num_bytes=m_size-static_cast<size_t>(m_offset-m_buffer);
		auto pointer=static_cast<void*>(m_offset);
		const auto aligned_address=std::align(alignof(T),sizeof(T),pointer,remaining_num_bytes);
		if (aligned_address==nullptr) {
			throw std::bad_alloc{};
		}
		m_offset=static_cast<std::byte*>(aligned_address)+sizeof(T);
		return static_cast<T*>(aligned_address);
	}

	~ArenaAllocator() {
		delete[] m_buffer;
	}

private:
	size_t m_size;
	std::byte* m_buffer;
	std::byte* m_offset;
};