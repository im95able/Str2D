#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>

#include "utility.h"
#include "flat_algorithm.h"

namespace seg
{

// Returns segment iterator of the given segment coordiante
template<typename C>
// C models SegmentCoordinate
SegmentIterator<C> segment(C& c) { return c.segment(); }

// Returns flat iterator of the given segment coordiante
template<typename C>
// C models SegmentCoordinate
FlatIterator<C> flat(C& c) { return c.flat(); }

template<typename I0, typename N, typename I1, typename C>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
std::pair<I0, std::pair<I1, FlatIterator<I1>>> _copy_flat_n_seg(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1, C c) {
	N n1 = static_cast<N>(std::end(first_seg1) - first_flat1);
	while(n0 > n1) {
		first0 = flat::_copy_n(first0, n1, first_flat1, c).first;
		n0 = n0 - n1;
		first_flat1 = std::begin(++first_seg1);
		n1 = static_cast<N>(std::end(first_seg1) - first_flat1);
	}
	return { flat::_copy_n(first0, n0, first_flat1, c).first, { first_seg1, flat::successor(first_flat1, n0) } };
}

template<typename I0, typename I1, typename C>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
std::pair<I1, FlatIterator<I1>> _copy_flat_seg(I0 first0, I0 last0, I1 first_seg1, FlatIterator<I1> first_flat1, C c) {
	if constexpr(RandomAccessIterator<I0>::value) {
		return seg::_copy_flat_n_seg(first0, last0 - first0, first_seg1, first_flat1, c).second;
	}
	else {
		if(first0 != last0) {
			FlatIterator<I1> last_flat1 = std::end(first_seg1);
			while(true) {
				c(*first_flat1++, *first0++);
				if (first_flat1 == last_flat1) 
					first_flat1 = std::begin(++first_seg1);
				if(first0 == last0) 
					return { first_seg1, first_flat1 };
			}
		}
	}
}

template<typename I0, typename I1, typename C>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
I1 _copy_seg_flat(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1, C c) {
	while (first_seg0 != last_seg0) {
		first1 = flat::_copy_n(first_flat0, std::end(first_seg0) - first_flat0, first1, c).second;
		first_flat0 = std::begin(++first_seg0);
	}
	return flat::_copy_n(first_flat0, last_flat0 - first_flat0, first1, c).second;
}

template<typename I0, typename I1, typename C>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
std::pair<I1, FlatIterator<I1>> _copy(
	I0 first_seg0, 
	FlatIterator<I0> first_flat0, 
	I0 last_seg0, 
	FlatIterator<I0> last_flat0, 
	I1 first_seg1, 
	FlatIterator<I1> first_flat1,
	C c) {
	
	if(first_seg0 != last_seg0) {
		while(true) {
			IteratorDifferenceType<FlatIterator<I0>> n0 = std::end(first_seg0) - first_flat0;
			IteratorDifferenceType<FlatIterator<I1>> n1 = std::end(first_seg1) - first_flat1;
			if(n0 > n1) {
				flat::_copy_n(first_flat0, n1, first_flat1, c);
				first_flat0 = flat::successor(first_flat0, n1);
				first_flat1 = std::begin(++first_seg1);
			}
			flat::_copy_n(first_flat0, n0, first_flat1, c);
			first_flat0 = std::begin(++first_seg0);
			if(n0 < n1) 
				first_flat1 = flat::successor(first_flat1, n0);
			else       
				first_flat1 = std::begin(++first_seg1);
			if (first_seg0 == last_seg0) break;
			
		}
	}
	return seg::_copy_flat_n_seg(first_flat0, last_flat0 - first_flat0, first_seg1, first_flat1, c).second;
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, std::pair<I1, FlatIterator<I1>>> copy_flat_n_seg(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_n_seg(first0, n0, first_seg1, first_flat1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_flat_seg(I0 first0, I0 last0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_seg(first0, last0, first_seg1, first_flat1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_seg_flat(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1) {
	return seg::_copy_seg_flat(first_seg0, first_flat0, last_seg0, last_flat0, first1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1) {
	return seg::_copy(first_seg0, first_flat0, last_seg0, last_flat0, first_seg1, first_flat1, copy_assignment());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_n_seg(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), copy_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_seg(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), copy_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I copy_seg_flat(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, copy_assignment());
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// IteratorValueType<C0> == IteratorValueType<C1>
inline
std::pair<C0, C1> copy(C0 first0, C0 last0, C1 first1) {
	auto [r0, r1] = seg::_copy(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), copy_assignment());
	return { C(r0), C(r1) };
}


template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, std::pair<I1, FlatIterator<I1>>> move_flat_seg_n(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_n_seg(first0, n0, first_seg1, first_flat1, move_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_flat_seg(I0 first0, I0 last0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_seg(first0, last0, first_seg1, first_flat1, move_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_seg_flat(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1) {
	return seg::_copy_seg_flat(first_seg0, first_flat0, last_seg0, last_flat0, first1, move_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1) {
	return seg::_copy(first_seg0, first_flat0, last_seg0, last_flat0, first_seg1, first_flat1, move_assignment());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_n_seg(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), move_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_seg(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), move_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I move_seg_flat(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, move_assignment());
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// IteratorValueType<C0> == IteratorValueType<C1>
inline
std::pair<C0, C1> move(C0 first0, C0 last0, C1 first1) {
	auto [r0, r1] = seg::_copy(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), move_assignment());
	return { C(r0), C(r1) };
}


template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, std::pair<I1, FlatIterator<I1>>> move_flat_n_seg_uninitialized(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_n_seg(first0, n0, first_seg1, first_flat1, move_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_flat_seg_uninitialized(I0 first0, I0 last0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_seg(first0, last0, first_seg1, first_flat1, move_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_seg_flat_uninitialized(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1) {
	return seg::_copy_seg_flat(first_seg0, first_flat0, last_seg0, last_flat0, first1, move_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_uninitialized(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1) {
	return seg::_copy(first_seg0, first_flat0, last_seg0, last_flat0, first_seg1, first_flat1, move_constructor());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_n_seg_uninitialized(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), move_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_seg_uninitialized(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), move_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I move_seg_flat_uninitialized(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, move_constructor());
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// IteratorValueType<C0> == IteratorValueType<C1>
inline
std::pair<C0, C1> move_uninitialized(C0 first0, C0 last0, C1 first1) {
	auto [r0, r1] = seg::_copy(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), move_constructor());
	return { C(r0), C(r1) };
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, std::pair<I1, FlatIterator<I1>>> copy_flat_n_seg_uninitialized(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_n_seg(first0, n0, first_seg1, first_flat1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_flat_seg_uninitialized(I0 first0, I0 last0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::_copy_flat_seg(first0, last0, first_seg1, first_flat1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_seg_flat_uninitialized(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1) {
	return seg::_copy_seg_flat(first_seg0, first_flat0, last_seg0, last_flat0, first1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_uninitialized(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1) {
	return seg::_copy(first_seg0, first_flat0, last_seg0, last_flat0, first_seg1, first_flat1, copy_constructor());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_n_seg_uninitialized(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), copy_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_seg_uninitialized(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), copy_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I copy_seg_flat_uninitialized(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, copy_constructor());
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// IteratorValueType<C0> == IteratorValueType<C1>
inline
std::pair<C0, C1> copy_uninitialized(C0 first0, C0 last0, C1 first1) {
	auto [r0, r1] = seg::_copy(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), copy_constructor());
	return { C(r0), C(r1) };
}



template<typename I, typename Proc>
// I models SegmentIterator
// Proc models Procedure
// Domain<Proc> == IteratorValueType<I>
Proc for_each(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, Proc p) {
	while (first_seg != last_seg) {
		IteratorDifferenceType<FlatIterator<I>> n = std::end(first_seg) - first_flat;
		while (n) {
			p(*first_flat);
			++first_flat;
			--n;
		}
		first_flat = std::begin(++first_seg);
	}
	return std::for_each(first_flat, last_flat, p);
}

template<typename C, typename Proc>
// C models SegmentCoordinate
// Proc models Procedure
// Domain<Proc> == IteratorValueType<C>
inline
Proc for_each(C first, C last, Proc p) {
	return seg::for_each(segment(first), flat(first), segment(last), flat(last), p);
}

template<typename I, typename T>
// I models SegmentIterator
// T == IteratorValueType<I>
void fill(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, const T& x) {
	while (first_seg != last_seg) {
		std::fill(first_flat, std::end(first_seg), x);
		first_flat = std::begin(++first_seg);
	}
	std::fill(first_flat, std::end(first_seg), x);
}

template<typename C, typename T>
// C models SegmentCoordinate
// Proc models Procedure
// T == IteratorValueType<I>
inline
void fill(C first, C last, const T& x) {
	return seg::fill(segment(first), flat(first), segment(last), flat(last), x);
}

template<typename I, typename T>
// I models SegmentIterator
// T == IteratorValueType<I>
void fill_uninitialized(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, const T& x) {
	while (first_seg != last_seg) {
		std::fill(first_flat, std::end(first_seg), x);
		first_flat = std::begin(++first_seg);
	}
	std::uninitialized_fill(first_flat, std::end(first_seg), x);
}

template<typename C, typename T>
// C models SegmentCoordinate
// Proc models Procedure
// T == IteratorValueType<I>
inline
void fill_uninitialized(C first, C last, const T& x) {
	return seg::fill_uninitialized(segment(first), flat(first), segment(last), flat(last), x);
}

template<typename I, typename Pred>
// I models SegmentIterator
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
std::pair<I, FlatIterator<I>> find_if(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, Pred p) {
	while (first_seg != last_seg) {
		FlatIterator<I> _last_flat = std::end(first_seg);
		FlatIterator<I> it = std::find_if(first_flat, _last_flat, p);
		if (it != _last_flat)
			return { first_seg, it };
		first_flat = std::begin(++first_seg);
	}
	return { first_seg, std::find_if(first_flat, last_flat, p) };
}

template<typename I, typename Pred>
// I models SegmentIterator
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
std::pair<I, FlatIterator<I>> find_if_not(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, Pred p) {
	return seg::find_if(first_seg, first_flat, last_seg, last_flat, unary_negate<Pred>(p));
}

template<typename C, typename Pred>
// C models SegmentCoordinate
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
inline
C find_if(C first, C last, Pred p) {
	return C(seg::find_if(segment(first), flat(first), segment(last), flat(last), p));
}

template<typename C, typename Pred>
// C models SegmentCoordinate
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
inline
C find_if_not(C first, C last, Pred p) {
	return C(seg::find_if_not(segment(first), flat(first), segment(last), flat(last), p));
}


template<typename I, typename Pred, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == Pred
// Codomain<Proc> == FlatIterator<I>
std::pair<I, FlatIterator<I>> partition_point(
	I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, Pred p, Proc pr = Proc{}) {
	IteratorDifferenceType<I> n = std::distance(first_seg, last_seg);
	while(n) {
		IteratorDifferenceType<I> h = n >> 1;
		I middle_seg = flat::successor(first_seg, h);
		FlatIterator<I> flat = flat::predecessor(std::end(middle_seg), 1);
		if(p(*flat)) {
			first_seg = flat::successor(middle_seg, 1);
			first_flat = std::begin(first_seg);
			n = n - h - 1;
		}
		else {
			last_seg = middle_seg;
			last_flat = flat;
			n = h;
		}
	}
	return { first_seg, pr(first_flat, last_flat, p) };
}

template<typename I, typename Pred, typename N, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<I>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == Pred
// Codomain<Proc> == FlatIterator<I>
inline
std::pair<I, FlatIterator<I>> partition_point_combined(
	I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, Pred p, N n, Proc pr = Proc{}) {

	if (first_seg - last_seg <= n) 
		return seg::find_if(first_seg, first_flat, last_seg, last_flat, p);
	
	if (n > 0) {
		I _last_seg = first_seg + n;
		FlatIterator<I> _last_flat = std::begin(_last_seg);
		std::tie(first_seg, first_flat) = seg::find_if(first_seg, first_flat, _last_seg, _last_flat, p);
		if (first_seg != _last_seg) return { first_seg, first_flat };
		if (first_flat != _last_flat) return { first_seg, first_flat };
	}
	return seg::partition_point(first_seg, first_flat, last_seg, last_flat, p, pr);
}

template<typename C, typename Pred, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == Pred
// Codomain<Proc> == FlatIterator<C>
inline
C partition_point(C first, C last, Pred p, Proc pr = Proc{}) {
	return C(seg::partition_point(segment(first), flat(first), segment(last), flat(last), p, pr));
}

template<typename C, typename Pred, typename N, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == Pred
// Codomain<Proc> == FlatIterator<C>
inline
C partition_point_combined(C first, C last, Pred p, N n, Proc pr = Proc{}) {
	return C(seg::partition_point_combined(segment(first), flat(first), segment(last), flat(last), p, n, pr));
}

template<typename I, typename Cmp, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<I>
// Codomain<Proc> == FlatIterator<I>
inline
std::pair<I, FlatIterator<I>> lower_bound(
	I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
	return seg::partition_point(first_seg, first_flat, last_seg, last_flat, lower_bound_predicate(x, cmp), pr);
}

template<typename I, typename Cmp, typename N, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<I>
// Codomain<Proc> == FlatIterator<I>
inline
std::pair<I, FlatIterator<I>> lower_bound_combined(
	I first_seg, 
	FlatIterator<I> first_flat, 
	I last_seg, 
	FlatIterator<I> last_flat, 
	const IteratorValueType<I>& x, 
	Cmp cmp, 
	N n, 
	Proc pr = Proc{}) 
{
	return seg::partition_point_combined(first_seg, first_flat, last_seg, last_flat, lower_bound_predicate(x, cmp), n, pr);
}

template<typename C, typename Cmp, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<C>
// Codomain<Proc> == FlatIterator<C>
inline
C lower_bound(C first, C last, const IteratorValueType<C>& x, Cmp cmp, Proc pr = Proc{}) {
	return C(seg::lower_bound(segment(first), flat(first), segment(last), flat(last), x, cmp, pr));
}

template<typename C, typename Cmp, typename N, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<C>
// Codomain<Proc> == FlatIterator<C>
inline
C lower_bound_combined(C first, C last, const IteratorValueType<C>& x, Cmp cmp, N n, Proc pr = Proc{}) {
	return C(seg::lower_bound_combined(segment(first), flat(first), segment(last), flat(last), x, cmp, n, pr));
}


template<typename I, typename Cmp, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<I>
// Codomain<Proc> == FlatIterator<I>
inline
std::pair<I, FlatIterator<I>> upper_bound(
	I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
	return seg::partition_point(first_seg, first_flat, last_seg, last_flat, upper_bound_predicate(x, cmp), pr);
}

template<typename I, typename Cmp, typename N, typename Proc = flat::find_adaptor_binary>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<I>
// Codomain<Proc> == FlatIterator<I>
inline
std::pair<I, FlatIterator<I>> upper_bound_combined(
	I first_seg, 
	FlatIterator<I> first_flat, 
	I last_seg, 
	FlatIterator<I> last_flat, 
	const IteratorValueType<I>& x, 
	Cmp cmp, 
	N n,
	Proc pr = Proc{}) {
	return seg::partition_point_combined(first_seg, first_flat, last_seg, last_flat, upper_bound_predicate(x, cmp), n, pr);
}

template<typename C, typename Cmp, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<C>
// Codomain<Proc> == FlatIterator<C>
inline
C upper_bound(C first, C last, const IteratorValueType<C>& x, Cmp cmp, Proc pr = Proc{}) {
	return C(seg::upper_bound(segment(first), flat(first), segment(last), flat(last), x, cmp, pr));
}

template<typename C, typename Cmp, typename N, typename Proc = flat::find_adaptor_binary>
// C models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// N models Integer
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == UnaryPredicate, Domain<UnaryPredicate> == IteratorValueType<C>
// Codomain<Proc> == FlatIterator<C>
inline
C upper_bound_combined(C first, C last, const IteratorValueType<C>& x, Cmp cmp, N n, Proc pr = Proc{}) {
	return C(seg::upper_bound_combined(segment(first), flat(first), segment(last), flat(last), x, cmp, n, pr));
}

template<typename I, typename Cmp, typename Proc = flat::equal_range_adaptor_binary>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<I>
// ArgumentType<Proc, 2> == Cmp
// Codomain<Proc> == std::pair<FlatIterator<I>, FlatIterator<I>>
pair2<I, FlatIterator<I>> equal_range(
	I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
		
	IteratorDifferenceType<I> n = std::distance(first_seg, last_seg);
	while(n) {
		IteratorDifferenceType<I> h = n >> 1;
		I middle_seg = std::next(first_seg, h);
		FlatIterator<I> flat = flat::predecessor(std::end(middle_seg), 1);
		if(cmp(x, *flat)) {
			last_seg = middle_seg;
			last_flat = flat;
			n = h;
		}
		else if(cmp(*flat, x)) {
			first_seg = flat::successor(middle_seg, 1);
			first_flat = std::begin(first_seg);
			n = n - h - 1;
		}
		else {
			I _middle_seg = flat::successor(middle_seg, 1);
			return { seg::lower_bound(first_seg, first_flat, middle_seg, flat, x, cmp), 
					 seg::upper_bound(_middle_seg, std::begin(_middle_seg), last_seg, last_flat, x, cmp) };
		}
	}
	
	std::pair<FlatIterator<I>, FlatIterator<I>> tmp = pr(first_flat, last_flat, x, cmp);
	return { {first_seg, tmp.first}, {first_seg, tmp.second} };
}

template<typename C, typename Cmp, typename Proc = flat::equal_range_adaptor_binary>
// C models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
// Proc models Procedure
// Arity<Proc> == 3
// ArgumentType<Proc, 0> == ArgumentType<Proc, 1> == FlatIterator<C>
// ArgumentType<Proc, 2> == AbstractUnaryPredicate, Domain<AbstractUnaryPredicate> == IteratorValueType<C>
// Codomain<Proc> == FlatIterator<C>
inline
std::pair<C, C> equal_range(C first, C last, const IteratorValueType<C>& x, Cmp cmp, Proc pr = Proc{}) {
	pair2<SegmentIterator<C>, FlatIterator<C>> tmp = seg::equal_range(segment(first), flat(first), segment(last), flat(last), x, cmp, pr);
	return { C(tmp.first), C(tmp.second) };
}


template<typename I>
// I models SegmentIterator
inline
void destruct(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while (first_seg != last_seg) {
			flat::destruct_n(first_flat, std::end(first_seg) - first_flat);
			first_flat = std::begin(++first_seg);
		}
		flat::destruct_n(first_flat, last_flat - first_flat);
	}
}

template<typename C>
// C models SegmentCoordinate
inline
void destruct(C first, C last) {
	return seg::destruct(segment(first), flat(first), segment(last), flat(last));
}


template<typename I0, typename I1, typename P>
// I0 models SegmentIterator
// I1 models ForwardIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal_seg_flat(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1, P p) {
	while (first_seg0 != last_seg0) {
		std::tie(first_flat0, first1) = flat::equal(first_flat0, std::end(first_seg0), first1, p);
		if (first_flat0 != std::end(first_seg0)) 
			return false;
		first_flat0 = std::begin(++first_seg0);
	}
	return flat::equal(first_flat0, std::end(first_seg0), first1, p);
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models ForwardIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// IteratorValueType<I0> models Regular
bool equal_seg_flat(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first1) {
	return seg::equal_seg_flat(first_seg0, first_flat0, last_seg0, last_flat0, first1, std::equal_to<>());
}

template<typename C, typename I, typename P>
// C models SegmentCoordinate
// I models InputIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<C> == IteratorValueType<I>
bool equal_seg_flat(C first0, C last0, I first1, P p) {
	return seg::equal_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), p);
}

template<typename C, typename I>
// C models SegmentCoordinate
// I models InputIterator
// IteratorValueType<C> == IteratorValueType<I>
bool equal_seg_flat(C first0, C last0, I first1) {
	return seg::equal_seg_flat(first0, last0, first1, std::equal_to<>());
}


template<typename I0, typename N, typename I1, typename P>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal_flat_n_seg(I0 first0, N n0, I1 first_seg1, FlatIterator<I1> first_flat1, I1 last_seg1, FlatIterator<I1> last_flat1, P p) {
	while(first_seg1 != last_seg1) {
		N n1 = static_cast<N>(std::end(first_seg1) - first_flat1);
		if (n0 < n1 || !flat::equal_n(first0, n1, first_flat1))
			return false;
		n0 = n0 - n1;
		first0 = flat::successor(first0, n1);
		first_flat1 = std::begin(++first_seg1);
 	}

	N n1 = static_cast<N>(last_flat1 - first_flat1);
	if (n0 < n1 || n0 > n1)
		return false;
	return flat::equal_n(first0, n0, first_flat1, p);
}

template<typename I0, typename N, typename I1>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// IteratorValueType<I0> models Regular
bool equal_flat_n_seg(I0 first0, N n, I1 first_seg1, FlatIterator<I1> first_flat1, I1 last_seg1, FlatIterator<I1> last_flat1) {
	return seg::equal_flat_n_seg(first0, n, first_seg1, first_flat1, last_seg1, last_flat1, std::equal_to<>());
}

template<typename I, typename N, typename C, typename P>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentCoordinate
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal_flat_n_seg(I first0, N n, C first1, C last1, P p) {
	return seg::equal_flat_n_seg(first0, n, segment(first1), flat(first1), segment(last1), flat(last1), p);
}

template<typename I, typename N, typename C>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentCoordinate
// IteratorValueType<I0> == IteratorValueType<I1>
// IteratorValueType<I0> models Regular
bool equal_flat_n_seg(I first0, N n, C first1, C last1) {
	return seg::equal_flat_n_seg(first0, n, first1, last1, std::equal_to<>());
}





template<typename I0, typename I1, typename P>
// I0 models SegmentIterator
// I1 models SegmentIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first_seg1, FlatIterator<I1> first_flat1, P p) {
	
	using Flat0 = FlatIterator<I0>;
	using Diff0 = IteratorDifferenceType<Flat0>;

	while (first_seg0 != last_seg0) {
		Diff0 n0 = std::end(first_seg0) - first_flat0;
		Diff0 n1 = static_cast<Diff0>(std::end(first_seg1) - first_flat1);
		if (n0 < n1) {
			if (!flat::equal_n(first_flat0, n0, first_flat1, p))
				return false;
			first_flat0 = std::begin(++first_seg0);
			first_flat1 = flat::successor(first_flat1, n0);
		}
		if (!flat::equal_n(first_flat0, n1, first_flat1, p))
			return false;

		first_flat1 = std::begin(++first_seg1);
		if (n0 > n1)
			first_flat0 = flat::successor(first_flat0, n1);
		else
			first_flat0 = std::begin(++first_seg0);
	}
	return seg::equal_flat_n_seg(first_flat0, last_flat0 - first_flat0, first_seg1, first_flat1, p);
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
bool equal(I0 first_seg0, FlatIterator<I0> first_flat0, I0 last_seg0, FlatIterator<I0> last_flat0, I1 first_seg1, FlatIterator<I1> first_flat1) {
	return seg::equal(first_seg0, first_flat0, last_seg0, last_flat1, first_seg1, first_flat1, std::equal_to<>());
}

template<typename C0, typename C1, typename P>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<C0> == IteratorValueType<C1>
bool equal(C0 first0, C0 last0, C1 first1, P p) {
	return seg::equal(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), p);
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<C0> == IteratorValueType<C1>
bool equal(C0 first0, C0 last0, C1 first1) {
	return seg::equal(first0, last0, first1, std::equal_to<>());
}


template<typename Cmp>
// Cmp models StrictWeakOrdering
struct equivalence_from_cmp
{
	Cmp cmp;

	equivalence_from_cmp(Cmp cmp) : cmp(cmp) {}

	template<typename T>
	// T == Domain<Cmp>
	bool operator()(const T& x, const T& y) const {
		return !(cmp(x, y) || cmp(y, x));
	}
};

template<typename I0, typename N, typename I1, typename Cmp>
// I0 models SegmentIterator
// I1 models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
int compare_flat_n_seg(I0 first0, N n0, I1 first_seg1, I1 first_flat1, I1 last_seg1, I1 last_flat1, Cmp cmp) {
	while (first_seg1 != last_seg1) {
		N n1 = static_cast<N>(std::end(first_seg1) - first_flat1);
		int r = flat::compare_n(first0, std::min(n0, n1), first_flat1, cmp);
		if (r != 0) return r;
		if (n0 <= n1) return -1;
		n0 = n0 - n1;
		first0 = flat::successor(first0, n1);
		first_flat1 = std::begin(++first_seg1);
	}
	N n1 = static_cast<N>(last_flat1 - first_flat1);
	int r = flat::compare_n(first0, std::min(n0, n1), first_flat1, cmp);
	if (r != 0) return r;
	if (n0 <= n1) return -1;
	return 1;
}

template<typename I0, typename I1, typename Cmp>
// I0 models SegmentIterator
// I1 models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
int compare(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1,
	I1 last_seg1,
	FlatIterator<I1> last_flat1,
	Cmp cmp) {

	using Flat0 = FlatIterator<I0>;
	using Diff0 = IteratorDifferenceType<Flat0>;
	using Flat1 = FlatIterator<I1>;
	using Diff1 = IteratorDifferenceType<Flat1>;

	while (first_seg0 != last_seg0 && first_seg1 != last_seg0) {
		Diff0 n0 = std::end(first_seg0) - first_flat0;
		Diff0 n1 = static_cast<Diff0>(std::end(first_seg1) - first_flat1);
		int r = flat::equal_n(first0, std::min(n0, n1), first_flat1);
		if (r != 0)
			return r;
		if (n0 < n1) {
			first_flat0 = std::begin(++first_seg0);
			first_flat1 = flat::successor(first_flat1, n0);
		}
		first_flat1 = std::begin(++first_seg1);
		if (n0 > n1)
			first_flat0 = flat::successor(first_flat0, n1);
		else
			first_flat0 = std::begin(++first_seg0);
	}

	if (first_seg0 == last_seg0) {
		Diff0 n0 = last_flat0 - first_flat0;
		if (first_seg1 == last_seg1) {
			Diff0 n1 = static_cast<Diff0>(last_flat1 - first_flat1);
			int r = flat::compare_n(first0, std::min(n0, n1), first_flat1, cmp);
			return r == 0 ? n0 < n1 : r;
		}
		return seg::compare_flat_n_seg(first_flat0, n0, first_seg1, first_flat1, last_seg1, last_flat1, cmp);
	}
	else {
		return -seg::compare_flat_n_seg(first_flat1, n1, first_seg0, first_flat0, last_seg0, last_flat0, cmp);
	}
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
int compare(
	I0 first_seg0,
	FlatIterator<I0> first_flat0,
	I0 last_seg0,
	FlatIterator<I0> last_flat0,
	I1 first_seg1,
	FlatIterator<I1> first_flat1,
	I1 last_seg1,
	FlatIterator<I1> last_flat1) {
	return seg::compare(first_seg0, first_flat0, last_seg0, last_flat0, first_seg1, first_flat1, last_seg1, last_flat1, std::less<>());
}

template<typename C0, typename C1, typename Cmp>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I0> == IteratorValueType<I1>
int compare(
	C0 first0, C0 last0, C1 first1, C1 last1, Cmp cmp) {
	return seg::compare(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), segment(last1), flat(last1), cmp);
}

template<typename C0, typename C1>
// C0 models SegmentCoordinate
// C1 models SegmentCoordinate
// IteratorValueType<I0> == IteratorValueType<I1>
int compare(
	C0 first0, C0 last0, C1 first1, C1 last1) {
	return seg::compare(first0, last0, first1, last1, std::less<>());
}




template<typename I>
// I models SegmentIterator
std::size_t distance(I first_seg, FlatIterator<I> first_flat, I last_seg, FlatIterator<I> last_flat) {
	std::size_t d = 0;
	while (first_seg != last_seg) {
		d = d + static_cast<std::size_t>(std::end(first_seg) - first_flat);
		first_flat = std::begin(++first_seg);
	}
	return d + static_cast<std::size_t>(last_flat - first_flat);
}

template<typename C>
// C models SegmentCoordinate
IteratorDifferenceType<C> distance(C first, C last) {
	return static_cast<IteratorDifferenceType<C>>(seg::distance(segment(first), flat(first), segment(last), flat(last)));
}


template<typename I, typename N>
// I models SegmentIterator
// N models Integer
inline
std::pair<I, FlatIterator<I>> successor(I first_seg, FlatIterator<I> first_flat, N n) {
	using DiffType = std::ptrdiff_t;

	DiffType _n = static_cast<DiffType>(n);
	FlatIterator<I> last_flat = std::end(first_seg);
	DiffType d = last_flat - first_flat;
	while (_n >= d && _n > 0) {
		_n = _n - d;
		first_flat = std::begin(++first_seg);
		d = std::end(first_seg) - first_flat;
	}
	return { first_seg, flat::successor(first_flat, _n) };
}

template<typename C, typename N>
// C models SegmentCoordinate
// N models Integer
inline
C successor(C first, N n) {
	return C(seg::successor(segment(first), flat(first), n));
}

template<typename I, typename N>
// I models SegmentIterator
// N models Integer
inline
std::pair<I, FlatIterator<I>> predecessor(I last_seg, FlatIterator<I> last_flat, N n) {
	using DiffType = std::ptrdiff_t;

	DiffType _n = static_cast<DiffType>(n);
	FlatIterator<I> first_flat = std::begin(last_seg);
	DiffType d = last_flat - first_flat;
	while (_n > d) {
		_n = _n - d;
		last_flat = std::end(--last_flat);
		d = last_flat - std::begin(last_flat);
	}
	return { last_flat,  flat::predecessor(last_flat, _n) };
}

template<typename C, typename N>
// C models SegmentCoordinate
// N models Integer
inline
C predecessor(C first, N n) {
	return C(seg::predecessor(segment(first), flat(first), n));
}

} // namespace seg

