#pragma once

#include <vector>
#include <algorithm>

class pool_allocator
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

	void deallocate_all() { std::for_each(std::begin(chunks), std::end(chunks), [](byte* ptr) { std::free(ptr); }); }

	void move_from(pool_allocator&& other) {
		free_list = other.free_list;
		free_list_end = other.free_list_end;
		block_size = other.block_size;
		chunk_size = other.chunk_size;
		chunk_blocks = other.chunk_blocks;
		chunks = std::move(other.chunks);
		other.chunks = std::vector<byte*>();
	}

	void copy_from(const pool_allocator& other) {
		free_list = nullptr;
		free_list_end = nullptr;
		block_size = other.block_size;
		chunk_size = other.chunk_size;
		chunk_blocks = other.chunk_blocks;
	}

public:
	pool_allocator(std::size_t block_size, std::size_t chunk_blocks, std::size_t reserve_chunks = 0) :
		block_size(std::max(block_size, min_block_size)),
		chunk_blocks(chunk_blocks),
		free_list(nullptr),
		free_list_end(nullptr)
	{
		chunk_size = chunk_blocks * block_size;
		for(std::size_t i = 0; i < reserve_chunks; ++i) add_chunk();
	}
	pool_allocator(const pool_allocator& other) { copy_from(other); }
	pool_allocator(pool_allocator&& other) noexcept { move_from(std::move(other)); }
	pool_allocator& operator=(const pool_allocator& other) { 
		deallocate_all();
		copy_from(other);
		return *this;
	}
	pool_allocator& operator=(pool_allocator&& other) noexcept {
		deallocate_all();
		move_from(std::move(other));
		return *this;
	}

	friend
	bool operator==(const pool_allocator& x, const pool_allocator y) {
		return x.block_size == y.block_size;
	}
	friend
	bool operator!=(const pool_allocator& x, const pool_allocator y) {
		return !(x == y);
	}

	~pool_allocator() { deallocate_all(); }

	void* allocate() {
		if (free_list == nullptr) add_chunk();
		return static_cast<void*>(free_list_block());
	}

	void deallocate(void* ptr) {
		byte* _ptr = static_cast<byte*>(ptr);
		*pptr(_ptr) = free_list;
		free_list = _ptr;
		if (free_list_end == nullptr)
			free_list_end = _ptr;
	}


};