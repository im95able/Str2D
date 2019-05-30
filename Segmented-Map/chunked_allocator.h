#pragma once

#include <vector>
#include <algorithm>

class chunked_block_allocator
{
	using byte = char;
	constexpr static std::size_t min_block_size = sizeof(byte*);
	
	std::size_t block_size;
	std::size_t chunk_size;
	std::size_t chunk_blocks;
	std::vector<byte*> chunks;
	byte* free_list;
	byte* free_list_end;

private:
	byte** pptr(byte* ptr) {
		return reinterpret_cast<byte**>(ptr);
	}

	byte* free_list_block() {
		byte* ptr = free_list;
		free_list = *pptr(ptr);
		return ptr;
	}

	void add_chunk() {
		byte* ptr = static_cast<byte*>(std::malloc(chunk_size));
		chunks.push_back(ptr);
		if (free_list != nullptr) {
			*pptr(free_list_end) = ptr;
		}
		else {
			free_list = ptr;
		}
		for (std::size_t i = 0; i < chunk_blocks - 1; ++i) {
			*pptr(ptr) = ptr + block_size;
			ptr = ptr + block_size;
		}
		free_list_end = ptr;
		*pptr(free_list_end) = nullptr;
	}

public:
	chunked_block_allocator(std::size_t block_size, std::size_t chunk_blocks, bool reserve_chunk = false) :
		block_size(std::max(block_size, min_block_size)),
		chunk_blocks(chunk_blocks),
		free_list(nullptr),
		free_list_end(nullptr)
	{
		chunk_size = chunk_blocks * block_size;
		if(reserve_chunk) add_chunk();
	}

	~chunked_block_allocator() {
		std::for_each(std::begin(chunks), std::end(chunks), [](byte* ptr) { std::free(ptr); });
	}

	void* allocate_block() {
		if (free_list == nullptr) add_chunk();
		return static_cast<void*>(free_list_block());
	}

	void deallocate_block(void* ptr) {
		byte* _ptr = static_cast<byte*>(ptr);
		*pptr(_ptr) = free_list;
		free_list = _ptr;
		if (free_list_end == nullptr)
			free_list_end = _ptr;
	}


};