#pragma once

#include <cstdint>
#include <vector>
#include <utility>

#include "utility.h"
#include "seg_algorithm.h"

namespace seg
{ 

// Terminology for internal mechanisms of segmented data structures and algorithms

// "segment" - an array of initialized elements

// "data" -  an array which consists of several arrays in the given order:
// 1) "front range"   - array of uninitialized elements
// 2) "segment"		  - array of initialized elements  
// 3) "back range"    - array of uninitialized elements

// "area" - a region of memory in which "data" resides. Besides "data" it could have some meta data allocated with it

// "capacity" - the size of "data"
	
// "limit" - minimal number of elements each "segment" must hold; except "first", which may hold less. "limit" is equal to "capacity" / 2

// "i" - index which points the the positions inside the "segment" where we either want to insert or erase "n" elements;
//		 considering it's relative to "segment" and not to "data", so is the return "i" for any function which takes "i" as one of its inputs

// "next pos" - position after the erased positions

// For insertion:
// "pre range" - range [begin("segment"), begin("segment") + i) 
// "n range" - imaginary flat range of "n" elements which is to be inserted
// "post range" - range [begin("segment") + i, end("segment))

// For erasure
// "pre range" - range [begin("segment"), "next pos") 
// "post range" - range ["next pos", end("segment))

// "first" segment - segment where the first element of the segmented range resides

// "last" segment - segment where the last element of the segmented range resides

// "edge last" segment - segment after the "last" segment; it's always dereferencable and always has size zero.
//						 In the case the segmented range is empty "edge last" segment == "last" segment. 

// Type that will actually be stored in the header or area
using segment_size_t = std::uint16_t;

// Type which is used for all calculations(much easier to just have one large type than to constantly manually do conversions)
// All conversion will happen within the functions of header interface.
using size_t = std::uint64_t;

template<typename T, segment_size_t C>
// T models Regular
// N models Integer
struct big_segment_header
{
	using value_type = T;
	struct area_type { T data[C]; };
	static constexpr segment_size_t capacity = C;

	area_type* area;
	segment_size_t first;
	segment_size_t last;
};

template<typename T, segment_size_t C>
// T models Regular
inline
AreaType<big_segment_header<T, C>>* area(big_segment_header<T, C>& h) { return h.area; }

template<typename T, segment_size_t C>
// T models Regular
inline
const AreaType<big_segment_header<T, C>>* area(const big_segment_header<T, C>& h) { return h.area; }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_area(big_segment_header<T, C>& h, AreaType<big_segment_header<T, C>>* area) { h.area = area; }

template<typename T, segment_size_t C>
// T models Regular
inline
ValueType<big_segment_header<T, C>>* data(big_segment_header<T, C>& h) { return h.area->data; }

template<typename T, segment_size_t C>
// T models Regular
inline
const ValueType<big_segment_header<T, C>>* data(const big_segment_header<T, C>& h) { return h.area->data; }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_begin_index(big_segment_header<T, C>& h, size_t index) { h.first = static_cast<segment_size_t>(index); }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_end_index(big_segment_header<T, C>& h, size_t index) { h.last = static_cast<segment_size_t>(index); }

template<typename T, segment_size_t C>
// T models Regular
inline
size_t begin_index(const big_segment_header<T, C>& h) { return static_cast<size_t>(h.first); }

template<typename T, segment_size_t C>
// T models Regular
inline
size_t end_index(const big_segment_header<T, C>& h) { return static_cast<size_t>(h.last); }

template<typename T, segment_size_t C>
struct small_segment_header
{
	using value_type = T;
	struct area_type
	{
		segment_size_t first;
		segment_size_t last;
		T data[C];
	};
	static constexpr segment_size_t capacity = C;

	area_type* area{ nullptr };
};

template<typename T, segment_size_t C>
// T models Regular
inline
AreaType<small_segment_header<T, C>>* area(small_segment_header<T, C>& h) { return h.area; }

template<typename T, segment_size_t C>
// T models Regular
inline
const AreaType<small_segment_header<T, C>>* area(const small_segment_header<T, C>& h) { return h.area; }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_area(small_segment_header<T, C>& h, AreaType<small_segment_header<T, C>>* area) { h.area = area; }

template<typename T, segment_size_t C>
// T models Regular
inline
ValueType<small_segment_header<T, C>>* data(small_segment_header<T, C>& h) { return h.area->data; }

template<typename T, segment_size_t C>
// T models Regular
inline
const ValueType<small_segment_header<T, C>>* data(const small_segment_header<T, C>& h) { return h.area->data; }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_begin_index(small_segment_header<T, C>& h, size_t index) { h.area->first = static_cast<segment_size_t>(index); }

template<typename T, segment_size_t C>
// T models Regular
inline
void set_end_index(small_segment_header<T, C>& h, size_t index) { h.area->last = static_cast<segment_size_t>(index); }

template<typename T, segment_size_t C>
// T models Regular
inline
size_t begin_index(const small_segment_header<T, C>& h) { return static_cast<size_t>(h.area->first); }

template<typename T, segment_size_t C>
// T models Regular
inline
size_t end_index(const small_segment_header<T, C>& h) { return static_cast<size_t>(h.area->last); }


template<typename H>
// H models SegmentHeader
inline
void set_begin_end_indices(H& h, size_t i) {
	set_begin_index(h, i);
	set_end_index(h, i);
}

template<typename H>
// H models SegmentHeader
inline
size_t size(const H& h) { return end_index(h) - begin_index(h); }

template<typename H>
// H models SegmentHeader or SegmentMeta
inline
constexpr size_t capacity(const H& h) { return static_cast<size_t>(H::capacity); }

template<typename H>
// H models SegmentHeader
inline
size_t available(const H& h) { return capacity(h) - seg::size(h); }

template<typename H>
// H models SegmentHeader
inline
size_t front(const H& h) { return begin_index(h); }

template<typename H>
// H models SegmentHeader
inline
size_t back(const H& h) { return capacity(h) - end_index(h); }

template<typename H> 
// H models SegmentHeader
inline
ValueType<H>* begin(H& h) { return flat::successor(data(h), begin_index(h)); }

template<typename H>
// H models SegmentHeader
inline
ValueType<H>* end(H& h) { return flat::successor(data(h), end_index(h)); }

template<typename H>
// H models SegmentHeader
inline
const ValueType<H>* begin(const H& h) { return flat::successor(data(h), begin_index(h)); }

template<typename H>
// H models SegmentHeader
inline
const ValueType<H>* end(const H& h) { return flat::successor(data(h), end_index(h)); }

template<typename H>
// H models SegmentHeader
inline
void increase_begin_index(H& h, size_t increase) { set_begin_index(h, begin_index(h) + increase); }

template<typename H>
// H models SegmentHeader
inline
void decrease_begin_index(H& h, size_t decrease) { set_begin_index(h, begin_index(h) - decrease); }

template<typename H>
// H models SegmentHeader
inline
void increase_end_index(H& h, size_t increase) { set_end_index(h, end_index(h) + increase); }

template<typename H>
// H models SegmentHeader
inline
void decrease_end_index(H& h, size_t decrease) { set_end_index(h, end_index(h) - decrease); }

template<typename H>
// H models SegmentHeader
inline
bool empty(const H& h) { return begin_index(h) == end_index(h); }

inline
constexpr size_t limit(size_t c) { return c >> 1; }

template<typename H, std::enable_if_t<!std::is_integral_v<H>, int> = 0>
// H models SegmentHeader
inline
constexpr size_t limit(const H& h) { return limit(capacity(h)); }

template<typename I>
// I models SegmentIndex
inline size_t segment_capacity(const I& index) {
	return static_cast<size_t>(I::segment_capacity);
}


//************************************************************************
// SEGMENT ITERATOR
//************************************************************************
template<typename HeaderIt>
struct segment_iterator
{
	using header_iterator = HeaderIt;
	using header_type = IteratorValueType<header_iterator>;
	using flat_iterator = ValueType<header_type>*;
	using const_flat_iterator = const ValueType<header_type>*;
	using value_type = ValueType<header_type>;
	using difference_type = IteratorDifferenceType<header_iterator>;
	using size_type = segment_size_t;
	using pointer = flat_iterator;
	using reference = value_type&;
	using iterator_category = std::random_access_iterator_tag;

	header_iterator h;

	segment_iterator() = default;
	segment_iterator(const segment_iterator&) = default;
	segment_iterator(header_iterator h) : h(h) {}

	friend
	bool operator==(const segment_iterator& x, const segment_iterator& y) {
		return x.h == y.h;
	}

	friend
	bool operator!=(const segment_iterator& x, const segment_iterator& y) {
		return !(x.h == y.h);
	}

	friend
	bool operator<(const segment_iterator& x, const segment_iterator& y) {
		return x.h < y.h;
	}

	friend
	bool operator>=(const segment_iterator& x, const segment_iterator& y) {
		return !(x < y);
	}

	friend
	bool operator>(const segment_iterator& x, const segment_iterator& y) {
		return y < x;
	}

	friend
	bool operator<=(const segment_iterator& x, const segment_iterator& y) {
		return !(y < x);
	}

	flat_iterator begin() { return flat_iterator(seg::begin(*h)); }
	flat_iterator end() { return flat_iterator(seg::end(*h)); }
	const_flat_iterator cbegin() const { return const_flat_iterator(seg::begin(*h)); }
	const_flat_iterator cend() const  { return const_flat_iterator(seg::end(*h)); }
	const_flat_iterator begin() const { return cbegin(); }
	const_flat_iterator end() const { return cend(); }

	size_type size() const { return seg::size(*h); }

	segment_iterator& operator++() {
		++h;
		return *this;
	}
	segment_iterator operator++(int) {
		segment_iterator tmp = *this;
		++*this;
		return tmp;
	}
	segment_iterator& operator--() {
		--h;
		return *this;
	}
	segment_iterator operator--(int) {
		segment_iterator tmp = *this;
		--*this;
		return tmp;
	}
	segment_iterator operator+(difference_type n) const {
		return segment_iterator(h + n);
	}
	segment_iterator operator-(difference_type n) const {
		return *this + (-n);
	}

	friend
	difference_type operator-(const segment_iterator& x, const segment_iterator& y) {
		return difference_type(x.h - y.h);
	}
};


template<typename HeaderIt, typename ConstHeaderIt>
struct const_segment_iterator
{
	using _header_iterator = HeaderIt;
	using _segment_iterator = segment_iterator<_header_iterator>;
	using const_header_iterator = ConstHeaderIt;
	using header_type = IteratorValueType<const_header_iterator>;
	using flat_iterator = const ValueType<header_type>*;
	using const_flat_iterator = flat_iterator;
	using value_type = ValueType<header_type>;
	using difference_type = IteratorDifferenceType<const_header_iterator>;
	using size_type = segment_size_t;
	using pointer = flat_iterator;
	using reference = value_type&;
	using iterator_category = std::random_access_iterator_tag;

	const_header_iterator h;

	const_segment_iterator() = default;
	const_segment_iterator(const const_segment_iterator&) = default;
	const_segment_iterator(_segment_iterator it) : h(const_header_iterator(it.h)) {}
	explicit const_segment_iterator(const_header_iterator h) : h(h) {}
	explicit const_segment_iterator(_header_iterator h) : h(const_header_iterator(h)) {}

	friend
	bool operator==(const const_segment_iterator& x, const const_segment_iterator& y) {
		return x.h == y.h;
	}

	friend
	bool operator!=(const const_segment_iterator& x, const const_segment_iterator& y) {
		return !(x.h == y.h);
	}

	friend
	bool operator<(const const_segment_iterator& x, const const_segment_iterator& y) {
		return x.h < y.h;
	}

	friend
	bool operator>=(const const_segment_iterator& x, const const_segment_iterator& y) {
		return !(x < y);
	}

	friend
	bool operator>(const const_segment_iterator& x, const const_segment_iterator& y) {
		return y < x;
	}

	friend
	bool operator<=(const const_segment_iterator& x, const const_segment_iterator& y) {
		return !(y < x);
	}

	flat_iterator begin() { return flat_iterator(seg::begin(*h)); }
	flat_iterator end() { return flat_iterator(seg::end(*h)); }
	const_flat_iterator cbegin() const { return const_flat_iterator(seg::begin(*h)); }
	const_flat_iterator cend() const  { return const_flat_iterator(seg::end(*h)); }
	const_flat_iterator begin() const { return cbegin(); }
	const_flat_iterator end() const { return cend(); }

	size_type size() const { return seg::size(*h); }

	const_segment_iterator& operator++() {
		++h;
		return *this;
	}
	const_segment_iterator operator++(int) {
		const_segment_iterator tmp = *this;
		++*this;
		return tmp;
	}
	const_segment_iterator& operator--() {
		--h;
		return *this;
	}
	const_segment_iterator operator--(int) {
		const_segment_iterator tmp = *this;
		--*this;
		return tmp;
	}
	const_segment_iterator operator+(difference_type n) const {
		return const_segment_iterator(h + n);
	}
	const_segment_iterator operator-(difference_type n) const {
		return *this + (-n);
	}

	friend
	difference_type operator-(const const_segment_iterator& x, const const_segment_iterator& y) {
		return difference_type(x.h - y.h);
	}
};
//************************************************************************
// ~SEGMENT ITERATOR
//************************************************************************




//************************************************************************
// SEGMENT COORDIANTE
//************************************************************************
template<typename SegmentIt, typename ConstSegmentIt>
struct segment_coordinate
{
	using segment_iterator = SegmentIt;
	using const_segment_iterator = ConstSegmentIt;
	using flat_iterator = FlatIterator<segment_iterator>;
	using const_flat_iterator = FlatIterator<const_segment_iterator>;
	using value_type = IteratorValueType<segment_iterator>;
	using iterator_category = std::bidirectional_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using pointer = IteratorPointer<segment_iterator>;
	using reference = IteratorReference<segment_iterator>;
	using iterator_category = std::bidirectional_iterator_tag;

	segment_iterator _seg;
	flat_iterator _flat;

	segment_coordinate() = default;
	segment_coordinate(const segment_coordinate&) = default;
	segment_coordinate(segment_iterator s, flat_iterator f) : _seg(s), _flat(f) {}
	explicit segment_coordinate(const std::pair<segment_iterator, flat_iterator>& p) : _seg(p.first), _flat(p.second) {}

	friend
	bool operator==(const segment_coordinate& x, const segment_coordinate& y) {
		return x._seg == y._seg && x._flat == y._flat;
	}
	friend
	bool operator!=(const segment_coordinate& x, const segment_coordinate& y) {
		return !(x == y);
	}

	friend
	bool operator<(const segment_coordinate& x, const segment_coordinate& y) {
		if (x._seg < y._seg) return true;
		if (y._seg < x._seg) return false;
		return x._flat < y._flat;
	}
	friend
	bool operator>=(const segment_coordinate& x, const segment_coordinate& y) {
		return !(x < y);
	}
	friend
	bool operator>(const segment_coordinate& x, const segment_coordinate& y) {
		return y < x;
	}
	friend
	bool operator<=(const segment_coordinate& x, const segment_coordinate& y) {
		return !(y < x);
	}

	reference operator*() const { return reference(*_flat); }
	pointer operator->() const { return pointer(&**this); }

	segment_coordinate& operator++() {
		if (_flat == --std::end(_seg)) {
			++_seg;
			_flat = std::begin(_seg); // Reason why must "last" segment must be after the segment which holds the last element
		}
		else {
			++_flat;
		}
		return *this;
	}
	segment_coordinate operator++(int) {
		segment_coordinate tmp = *this;
		++*this;
		return tmp;
	}
	segment_coordinate& operator--() {
		if (_flat == std::begin(_seg)) {
			_flat = --std::end(--_seg);
		}
		else {
			--_flat;
		}
		return *this;
	}
	segment_coordinate operator--(int) {
		segment_coordinate tmp = *this;
		--*this;
		return tmp;
	}

	flat_iterator flat() { return _flat; }
	segment_iterator segment() { return _seg; }
	const_flat_iterator cflat() const { return const_flat_iterator(_flat); }
	const_segment_iterator csegment() const { return const_segment_iterator(_seg); }
	const_flat_iterator flat() const { return cflat(); }
	const_segment_iterator segment() const { return csegment(); }
};


template<typename SegmentIt, typename ConstSegmentIt>
struct const_segment_coordinate
{
	using _segment_iterator = SegmentIt;
	using _flat_iterator = FlatIterator<_segment_iterator>;
	using _segment_coordinate = segment_coordinate<SegmentIt, ConstSegmentIt>;
	using segment_iterator = ConstSegmentIt;
	using const_segment_iterator = ConstSegmentIt;
	using flat_iterator = FlatIterator<segment_iterator>;
	using const_flat_iterator = flat_iterator;
	using value_type = IteratorValueType<segment_iterator>;
	using iterator_category = std::bidirectional_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using pointer = IteratorPointer<segment_iterator>;
	using reference = IteratorReference<segment_iterator>;
	using iterator_category = std::bidirectional_iterator_tag;

	segment_iterator _seg;
	flat_iterator _flat;

	const_segment_coordinate() = default;
	const_segment_coordinate(const const_segment_coordinate&) = default;
	const_segment_coordinate(const _segment_coordinate& it) : _seg(const_segment_iterator(it._seg)), _flat(const_flat_iterator(it._flat)) {}
	const_segment_coordinate(const_segment_iterator s, const_flat_iterator f) : _seg(s), _flat(f) {}
	const_segment_coordinate(_segment_iterator s, _flat_iterator f) : _seg(const_segment_iterator(s)), _flat(const_flat_iterator(f)) {}
	explicit const_segment_coordinate(const std::pair<const_segment_iterator, const_segment_iterator>& p) : _seg(p.first), _flat(p.second) {}
	explicit const_segment_coordinate(std::pair<_segment_iterator, _flat_iterator>& p) : _seg(const_segment_iterator(p.first)), _flat(const_flat_iterator(p.second)) {}

	friend
	bool operator==(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		return x._seg == y._seg && x._flat == y._flat;
	}
	friend
	bool operator!=(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		return !(x == y);
	}

	friend
	bool operator<(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		if (x._seg < y._seg) return true;
		if (y._seg < x._seg) return false;
		return x._flat < y._flat;
	}
	friend
	bool operator>=(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		return !(x < y);
	}
	friend
	bool operator>(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		return y < x;
	}
	friend
	bool operator<=(const const_segment_coordinate& x, const const_segment_coordinate& y) {
		return !(y < x);
	}

	reference operator*() const { return reference(*_flat); }
	pointer operator->() const { return pointer(&**this); }

	const_segment_coordinate& operator++() {
		if (_flat == --std::end(_seg)) {
			++_seg;
			_flat = std::begin(_seg); // Reason why must "last" segment must be after the segment which holds the last element
		}
		else {
			++_flat;
		}
		return *this;
	}
	const_segment_coordinate operator++(int) {
		const_segment_coordinate tmp = *this;
		++* this;
		return tmp;
	}
	const_segment_coordinate& operator--() {
		if (_flat == std::begin(_seg)) {
			_flat = --std::end(--_seg);
		}
		else {
			--_flat;
		}
		return *this;
	}
	const_segment_coordinate operator--(int) {
		const_segment_coordinate tmp = *this;
		--* this;
		return tmp;
	}

	flat_iterator flat() { return _flat; }
	segment_iterator segment() { return _seg; }
	const_flat_iterator cflat() const { return const_flat_iterator(_flat); }
	const_segment_iterator csegment() const { return const_segment_iterator(_seg); }
	const_flat_iterator flat() const { return cflat(); }
	const_segment_iterator segment() const { return csegment(); }
};
//************************************************************************
// ~SEGMENT COORDIANTE
//************************************************************************

} // namespace seg
