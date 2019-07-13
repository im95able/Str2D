#pragma once

#include <vector>
#include <algorithm>

namespace str2d
{ 

template<std::size_t BLOCK_SIZE, std::size_t CHUNK_BLOCKS>
class pool_allocator_base
{
private:
	using byte = char;

	constexpr static std::size_t min_block_size = sizeof(byte*);
	constexpr static std::size_t block_size = std::max(BLOCK_SIZE, min_block_size);
	constexpr static std::size_t chunk_blocks = CHUNK_BLOCKS;
	constexpr static std::size_t chunk_size = CHUNK_BLOCKS * std::max(BLOCK_SIZE, min_block_size);

	std::vector<byte*> chunks;
	byte* free_list;
	byte* free_list_end;

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

	void move_from(pool_allocator_base&& other) {
		free_list = other.free_list;
		free_list_end = other.free_list_end;
		chunks = std::move(other.chunks);
		other.chunks = std::vector<byte*>();
	}

	void copy_from(const pool_allocator_base& other) {
		free_list = nullptr;
		free_list_end = nullptr;
	}

public:
	pool_allocator_base(std::size_t reserve_chunks = 0) :
		free_list(nullptr),
		free_list_end(nullptr)
	{
		for(std::size_t i = 0; i < reserve_chunks; ++i) add_chunk();
	}
	pool_allocator_base(const pool_allocator_base& other) { copy_from(other); }
	pool_allocator_base(pool_allocator_base&& other) noexcept { move_from(std::move(other)); }
	pool_allocator_base& operator=(const pool_allocator_base& other) {
		deallocate_all();
		copy_from(other);
		return *this;
	}
	pool_allocator_base& operator=(pool_allocator_base&& other) noexcept {
		deallocate_all();
		move_from(std::move(other));
		return *this;
	}

	friend
	bool operator==(const pool_allocator_base& x, const pool_allocator_base y) {
		return true;
	}
	friend
	bool operator!=(const pool_allocator_base& x, const pool_allocator_base y) {
		return false;
	}

	~pool_allocator_base() { deallocate_all(); }

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


template<typename BLOCK_TYPE, std::size_t CHUNK_BLOCKS>
class pool_allocator
{
public:
	using value_type = BLOCK_TYPE;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using is_always_equal = std::true_type;

private:
	pool_allocator_base<sizeof(value_type), CHUNK_BLOCKS> alloc;

public:
	template<typename O>
	struct rebind {
		using other = pool_allocator<O, CHUNK_BLOCKS>;
	};

	pool_allocator(std::size_t reserve_chunks = 0) : alloc(reserve_chunks) {}

	pool_allocator(const pool_allocator& other) = default; 
	pool_allocator(pool_allocator&& other) = default;
	pool_allocator& operator=(const pool_allocator& other) = default;
	pool_allocator& operator=(pool_allocator&& other) = default;

	friend
	bool operator==(const pool_allocator& x, const pool_allocator y) {
		return x.alloc == y.alloc;
	}
	friend
		bool operator!=(const pool_allocator& x, const pool_allocator y) {
		return !(x == y);
	}

	value_type* allocate(size_type n) {
		return static_cast<value_type*>(alloc.allocate());
	}

	void deallocate(value_type* ptr, size_type n) {
		alloc.deallocate(static_cast<void*>(ptr));
	}


};

} // namespace str2d