#pragma once

#include <iterator>
#include <type_traits>
#include <memory>
#include <utility>
#include <cstdlib>

template<typename C>
// C models Container
using SizeType = typename C::size_type;

template<typename C>
// C models Container
using ValueType = typename C::value_type;

template<typename C>
// C models Container
using AllocatorType = typename C::allocator;

template<typename H>
// H models SegmentHeader
using AreaType = typename H::area_type;

template<typename I>
// I models Iterator
using IteratorValueType = typename std::iterator_traits<I>::value_type;

template<typename I>
// I models Iterator
using IteratorDifferenceType = typename std::iterator_traits<I>::difference_type;

template<typename I>
// I models Iterator
using IteratorPointer = typename std::iterator_traits<I>::pointer;

template<typename I>
// I models Iterator
using IteratorReference = typename std::iterator_traits<I>::reference;

template<typename I>
// I models Iterator
using IteratorCategory = typename std::iterator_traits<I>::iterator_category;

template<typename C>
// C models Container
using Iterator = typename C::iterator;

template<typename C>
// C models Container
using ConstIterator = typename C::const_iterator;

template<typename C>
// C models Container
using ReverseIterator = typename C::reverse_iterator;

template<typename C>
// C models Container
using ConstReverseIterator = typename C::const_reverse_iterator;

template<typename I>
// I models SegmentIterator or SegmentCoordinate
using FlatIterator = typename I::flat_iterator;

template<typename I>
// I models SegmentIterator or SegmentCoordinate
using ConstFlatIterator = typename I::const_flat_iterator;

template<typename C>
// C models SegmentCoordinate
using SegmentIterator = typename C::segment_iterator;

template<typename C>
// C models SegmentCoordinate
using ConstSegmentIterator = typename C::const_segment_iterator;


template<typename I1, typename I2>
using TriviallyCopyableMemory = std::integral_constant<bool,
	std::is_pointer<I1>::value &&
	std::is_pointer<I2>::value &&
	std::is_same<IteratorValueType<I1>, IteratorValueType<I2>>::value &&
	std::is_trivially_copyable<IteratorValueType<I1>>::value &&
	std::is_trivially_destructible<IteratorValueType<I1>>::value>;

template<typename T>
using TriviallyDestructible = std::integral_constant<bool, std::is_trivially_destructible<T>::value>;

template<typename I>
using RandomAccessIterator = std::integral_constant<bool, std::is_same<IteratorCategory<I>, std::random_access_iterator_tag>::value>;

template<typename T1, typename T2>
using pair2 = std::pair<std::pair<T1, T2>, std::pair<T1, T2>>;

template<typename I>
// I models Dereferencable
inline
void destruct_at(I it) { (*it).~IteratorValueType<I>(); }

template<typename I, typename... Args>
// I models Dereferencable
inline
void construct_at(I it, Args&&... args) { new(std::addressof(*it)) IteratorValueType<I>(std::forward<Args>(args)...); }

struct copy_constructor {
	template<typename T0, typename T1>
	// T1 is assignable to T0
	void operator()(T0& x, T1& y) { new(std::addressof(x)) T0(y); }
};

struct move_constructor {
	template<typename T0, typename T1>
	// T1 is assignable to T0
	void operator()(T0& x, T1& y) { new(std::addressof(x)) T0(std::move(y)); }
};

struct copy_assignment {
	template<typename T0, typename T1>
	// T1 is assignable to T0
	void operator()(T0& x, T1& y) { x = y; }
};

struct move_assignment {
	template<typename T0, typename T1>
	// T1 is assignable to T0
	void operator()(T0& x, T1& y) { x = std::move(y); }
};

template<typename N, typename M = N, typename Q = N, typename R = N>
inline
std::pair<Q, R> division_with_remainder(N numerator, M denominator) {
	std::ldiv_t r = std::ldiv(static_cast<long>(numerator), static_cast<long>(denominator));
	return { static_cast<Q>(r.quot), static_cast<R>(r.rem) };
}

template<typename P>
// P models Predicate
struct unary_negate
{
	P p;
	unary_negate(P p) : p(p) {}

	template<typename T>
	// T == Domain<P>
	bool operator()(const T& x) { return !p(x); }
};

template<typename T, typename Cmp>
// Cmp models StrictWeakOrdering
// Domain<Cmp> == T
struct lower_bound_predicate
{
	const T* x;
	Cmp cmp;

	lower_bound_predicate(const T& x, Cmp cmp) : x(&x), cmp(cmp) {}

	bool operator()(const T& y) const {
		return !cmp(y, *x);
	}
};

template<typename T, typename Cmp>
// Cmp models StrictWeakOrdering
// Domain<Cmp> == T
struct upper_bound_predicate
{
	const T* x;
	Cmp cmp;

	upper_bound_predicate(const T& x, Cmp cmp) : x(&x), cmp(cmp) {}

	bool operator()(const T& y) const {
		return cmp(*x, y);
	}
};

struct equal_to
{
	template<typename T0, typename T1>
	bool operator()(const T0& x, const T1& y) {
		return x == y;
	}
};

struct less
{
	template<typename T0, typename T1>
	bool operator()(const T0& x, const T1& y) {
		return x < y;
	}
};

struct less_equal
{
	template<typename T0, typename T1>
	bool operator()(const T0& x, const T1& y) {
		return x <= y;
	}
};

struct greater
{
	template<typename T0, typename T1>
	bool operator()(const T0& x, const T1& y) {
		return x > y;
	}
};

struct greater_equal
{
	template<typename T0, typename T1>
	bool operator()(const T0& x, const T1& y) {
		return x >= y;
	}
};