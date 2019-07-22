#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>

#include "utility.h"
#include "flat_algorithm.h"

namespace str2d
{ 

namespace seg
{

// Returns segment iterator of the given segmented coordinate
template<typename C>
// C models SegmentedCoordinate
SegmentIterator<C> segment(C& c) { return c.segment(); }

template<typename C>
// C models SegmentedCoordinate
ConstSegmentIterator<C> segment(const C& c) { return c.csegment(); }

template<typename C>
// C models SegmentedCoordinate
ConstSegmentIterator<C> csegment(const C& c) { return c.csegment(); }



// Returns flat iterator of the given segmented coordinate
template<typename C>
// C models SegmentedCoordinate
FlatIterator<C> flat(C& c) { return c.flat(); }

template<typename C>
// C models SegmentedCoordinate
ConstFlatIterator<C> flat(const C& c) { return c.cflat(); }

template<typename C>
// C models SegmentedCoordinate
ConstFlatIterator<C> cflat(const C& c) { return c.cflat(); }



// Returns both the flat and segment iterator of the given segmented coordinate
template<typename C>
// C models SegmentedCoordinate
std::pair<SegmentIterator<C>, FlatIterator<C>> extract(C& c) { c.extract(); }

template<typename C>
// C models SegmentedCoordinate
std::pair<ConstSegmentIterator<C>, ConstFlatIterator<C>> extract(const C& c) { c.cextract(); }

template<typename C>
// C models SegmentedCoordinate
std::pair<ConstSegmentIterator<C>, ConstFlatIterator<C>> cextract(const C& c) { c.cextract(); }

template<typename I0, typename N, typename I1, typename C>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
std::pair<I0, std::pair<I1, FlatIterator<I1>>> _copy_flat_n_seg(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1, C c) {
	N n1 = static_cast<N>(std::end(fseg1) - fflat1);
	while(n0 > n1) {
		first0 = flat::_copy_n(first0, n1, fflat1, c).first;
		n0 = n0 - n1;
		fflat1 = std::begin(++fseg1);
		n1 = static_cast<N>(std::end(fseg1) - fflat1);
	}
	return { flat::_copy_n(first0, n0, fflat1, c).first, { fseg1, flat::successor(fflat1, n0) } };
}

template<typename I0, typename I1, typename C>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I0>
// InputType<C, 1> == IteratorValueType<I1>
std::pair<I1, FlatIterator<I1>> _copy_flat_seg(I0 first0, I0 last0, I1 fseg1, FlatIterator<I1> fflat1, C c) {
	if constexpr(RandomAccessIterator<I0>::value) {
		return seg::_copy_flat_n_seg(first0, last0 - first0, fseg1, fflat1, c).second;
	}
	else {
		if(first0 != last0) {
			FlatIterator<I1> lflat1 = std::end(fseg1);
			while(true) {
				c(*fflat1++, *first0++);
				if (fflat1 == lflat1) 
					fflat1 = std::begin(++fseg1);
				if(first0 == last0) 
					return { fseg1, fflat1 };
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
I1 _copy_seg_flat(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1, C c) {
	while (fseg0 != lseg0) {
		first1 = flat::_copy_n(fflat0, std::end(fseg0) - fflat0, first1, c).second;
		fflat0 = std::begin(++fseg0);
	}
	return flat::_copy_n(fflat0, lflat0 - fflat0, first1, c).second;
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
	I0 fseg0, 
	FlatIterator<I0> fflat0, 
	I0 lseg0, 
	FlatIterator<I0> lflat0, 
	I1 fseg1, 
	FlatIterator<I1> fflat1,
	C c) {
	
	if(fseg0 != lseg0) {
		while(true) {
			IteratorDifferenceType<FlatIterator<I0>> n0 = std::end(fseg0) - fflat0;
			IteratorDifferenceType<FlatIterator<I1>> n1 = std::end(fseg1) - fflat1;
			if(n0 > n1) {
				flat::_copy_n(fflat0, n1, fflat1, c);
				fflat0 = flat::successor(fflat0, n1);
				fflat1 = std::begin(++fseg1);
			}
			flat::_copy_n(fflat0, n0, fflat1, c);
			fflat0 = std::begin(++fseg0);
			if(n0 < n1) 
				fflat1 = flat::successor(fflat1, n0);
			else       
				fflat1 = std::begin(++fseg1);
			if (fseg0 == lseg0) break;
			
		}
	}
	return seg::_copy_flat_n_seg(fflat0, lflat0 - fflat0, fseg1, fflat1, c).second;
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, std::pair<I1, FlatIterator<I1>>> copy_flat_n_seg(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_n_seg(first0, n0, fseg1, fflat1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_flat_seg(I0 first0, I0 last0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_seg(first0, last0, fseg1, fflat1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_seg_flat(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1) {
	return seg::_copy_seg_flat(fseg0, fflat0, lseg0, lflat0, first1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1) {
	return seg::_copy(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, copy_assignment());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_n_seg(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), copy_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_seg(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), copy_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I copy_seg_flat(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, copy_assignment());
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
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
std::pair<I0, std::pair<I1, FlatIterator<I1>>> move_flat_seg_n(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_n_seg(first0, n0, fseg1, fflat1, move_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_flat_seg(I0 first0, I0 last0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_seg(first0, last0, fseg1, fflat1, move_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_seg_flat(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1) {
	return seg::_copy_seg_flat(fseg0, fflat0, lseg0, lflat0, first1, move_assignment());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1) {
	return seg::_copy(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, move_assignment());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_n_seg(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), move_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_seg(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), move_assignment());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I move_seg_flat(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, move_assignment());
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
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
std::pair<I0, std::pair<I1, FlatIterator<I1>>> move_flat_n_seg_uninitialized(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_n_seg(first0, n0, fseg1, fflat1, move_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_flat_seg_uninitialized(I0 first0, I0 last0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_seg(first0, last0, fseg1, fflat1, move_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_seg_flat_uninitialized(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1) {
	return seg::_copy_seg_flat(fseg0, fflat0, lseg0, lflat0, first1, move_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> move_uninitialized(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1) {
	return seg::_copy(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, move_constructor());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_n_seg_uninitialized(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), move_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> move_flat_seg_uninitialized(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), move_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I move_seg_flat_uninitialized(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, move_constructor());
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
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
std::pair<I0, std::pair<I1, FlatIterator<I1>>> copy_flat_n_seg_uninitialized(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_n_seg(first0, n0, fseg1, fflat1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_flat_seg_uninitialized(I0 first0, I0 last0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::_copy_flat_seg(first0, last0, fseg1, fflat1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_seg_flat_uninitialized(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1) {
	return seg::_copy_seg_flat(fseg0, fflat0, lseg0, lflat0, first1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I1, FlatIterator<I1>> copy_uninitialized(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1) {
	return seg::_copy(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, copy_constructor());
}

template<typename I, typename N, typename C>
// I models InputIterator
// N models Integer
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_n_seg_uninitialized(I first0, N n0, C first1) {
	auto [r0, r1] = seg::_copy_flat_n_seg(first0, n0, segment(first1), flat(first1), copy_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models InputIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
std::pair<I, C> copy_flat_seg_uninitialized(I first0, I last0, C first1) {
	auto [r0, r1] = seg::_copy_flat_seg(first0, last0, segment(first1), flat(first1), copy_constructor());
	return { r0, C(r1) };
}

template<typename I, typename C>
// I models SegmentIterator
// C models SegmentedCoordinate
// IteratorValueType<I> == IteratorValueType<C>
inline
I copy_seg_flat_uninitialized(C first0, C last0, I first1) {
	return seg::_copy_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), first1, copy_constructor());
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
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
Proc for_each(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, Proc p) {
	while (fseg != lseg) {
		IteratorDifferenceType<FlatIterator<I>> n = std::end(fseg) - fflat;
		while (n) {
			p(*fflat);
			++fflat;
			--n;
		}
		fflat = std::begin(++fseg);
	}
	return std::for_each(fflat, lflat, p);
}

template<typename C, typename Proc>
// C models SegmentedCoordinate
// Proc models Procedure
// Domain<Proc> == IteratorValueType<C>
inline
Proc for_each(C first, C last, Proc p) {
	return seg::for_each(segment(first), flat(first), segment(last), flat(last), p);
}

template<typename I, typename T>
// I models SegmentIterator
// T == IteratorValueType<I>
void fill(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const T& x) {
	while (fseg != lseg) {
		std::fill(fflat, std::end(fseg), x);
		fflat = std::begin(++fseg);
	}
	std::fill(fflat, std::end(fseg), x);
}

template<typename C, typename T>
// C models SegmentedCoordinate
// Proc models Procedure
// T == IteratorValueType<I>
inline
void fill(C first, C last, const T& x) {
	return seg::fill(segment(first), flat(first), segment(last), flat(last), x);
}

template<typename I, typename T>
// I models SegmentIterator
// T == IteratorValueType<I>
void fill_uninitialized(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const T& x) {
	while (fseg != lseg) {
		std::fill(fflat, std::end(fseg), x);
		fflat = std::begin(++fseg);
	}
	std::uninitialized_fill(fflat, std::end(fseg), x);
}

template<typename C, typename T>
// C models SegmentedCoordinate
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
std::pair<I, FlatIterator<I>> find_if(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, Pred p) {
	while (fseg != lseg) {
		FlatIterator<I> _lflat = std::end(fseg);
		FlatIterator<I> it = std::find_if(fflat, _lflat, p);
		if (it != _lflat)
			return { fseg, it };
		fflat = std::begin(++fseg);
	}
	return { fseg, std::find_if(fflat, lflat, p) };
}

template<typename I, typename Pred>
// I models SegmentIterator
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
std::pair<I, FlatIterator<I>> find_if_not(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, Pred p) {
	return seg::find_if(fseg, fflat, lseg, lflat, unary_negate<Pred>(p));
}

template<typename C, typename Pred>
// C models SegmentedCoordinate
// Pred models UnaryPredicate
// Domain<Pred> == IteratorValueType<C>
inline
C find_if(C first, C last, Pred p) {
	return C(seg::find_if(segment(first), flat(first), segment(last), flat(last), p));
}

template<typename C, typename Pred>
// C models SegmentedCoordinate
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
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, Pred p, Proc pr = Proc{}) {
	IteratorDifferenceType<I> n = std::distance(fseg, lseg);
	while(n) {
		IteratorDifferenceType<I> h = n >> 1;
		I middle_seg = flat::successor(fseg, h);
		FlatIterator<I> flat = flat::predecessor(std::end(middle_seg), 1);
		if(p(*flat)) {
			fseg = flat::successor(middle_seg, 1);
			fflat = std::begin(fseg);
			n = n - h - 1;
		}
		else {
			lseg = middle_seg;
			lflat = flat;
			n = h;
		}
	}
	return { fseg, pr(fflat, lflat, p) };
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
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, Pred p, N n, Proc pr = Proc{}) {

	if (fseg - lseg <= n) 
		return seg::find_if(fseg, fflat, lseg, lflat, p);
	
	if (n > 0) {
		I _lseg = fseg + n;
		FlatIterator<I> _lflat = std::begin(_lseg);
		std::tie(fseg, fflat) = seg::find_if(fseg, fflat, _lseg, _lflat, p);
		if (fseg != _lseg) return { fseg, fflat };
		if (fflat != _lflat) return { fseg, fflat };
	}
	return seg::partition_point(fseg, fflat, lseg, lflat, p, pr);
}

template<typename C, typename Pred, typename Proc = flat::find_adaptor_binary>
// C models SegmentedCoordinate
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
// C models SegmentedCoordinate
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
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
	return seg::partition_point(fseg, fflat, lseg, lflat, lower_bound_predicate(x, cmp), pr);
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
	I fseg, 
	FlatIterator<I> fflat, 
	I lseg, 
	FlatIterator<I> lflat, 
	const IteratorValueType<I>& x, 
	Cmp cmp, 
	N n, 
	Proc pr = Proc{}) 
{
	return seg::partition_point_combined(fseg, fflat, lseg, lflat, lower_bound_predicate(x, cmp), n, pr);
}

template<typename C, typename Cmp, typename Proc = flat::find_adaptor_binary>
// C models SegmentedCoordinate
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
// C models SegmentedCoordinate
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



template<typename I, typename Cmp>
// I models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I>
inline
std::pair<I, FlatIterator<I>> upper_bound_linear(
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const IteratorValueType<I>& x, Cmp cmp) {
	return seg::partition_point_linear(fseg, fflat, lseg, lflat, upper_bound_predicate(x, cmp));
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
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
	return seg::partition_point(fseg, fflat, lseg, lflat, upper_bound_predicate(x, cmp), pr);
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
	I fseg, 
	FlatIterator<I> fflat, 
	I lseg, 
	FlatIterator<I> lflat, 
	const IteratorValueType<I>& x, 
	Cmp cmp, 
	N n,
	Proc pr = Proc{}) {
	return seg::partition_point_combined(fseg, fflat, lseg, lflat, upper_bound_predicate(x, cmp), n, pr);
}


template<typename C, typename Cmp, typename Proc = flat::find_adaptor_binary>
// C models SegmentedCoordinate
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
// C models SegmentedCoordinate
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
	I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat, const IteratorValueType<I>& x, Cmp cmp, Proc pr = Proc{}) {
		
	IteratorDifferenceType<I> n = std::distance(fseg, lseg);
	while(n) {
		IteratorDifferenceType<I> h = n >> 1;
		I middle_seg = std::next(fseg, h);
		FlatIterator<I> flat = flat::predecessor(std::end(middle_seg), 1);
		if(cmp(x, *flat)) {
			lseg = middle_seg;
			lflat = flat;
			n = h;
		}
		else if(cmp(*flat, x)) {
			fseg = flat::successor(middle_seg, 1);
			fflat = std::begin(fseg);
			n = n - h - 1;
		}
		else {
			I _middle_seg = flat::successor(middle_seg, 1);
			return { seg::lower_bound(fseg, fflat, middle_seg, flat, x, cmp), 
					 seg::upper_bound(_middle_seg, std::begin(_middle_seg), lseg, lflat, x, cmp) };
		}
	}
	
	std::pair<FlatIterator<I>, FlatIterator<I>> tmp = pr(fflat, lflat, x, cmp);
	return { {fseg, tmp.first}, {fseg, tmp.second} };
}

template<typename C, typename Cmp, typename Proc = flat::equal_range_adaptor_binary>
// C models SegmentedCoordinate
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
void destruct(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while (fseg != lseg) {
			flat::destruct_n(fflat, std::end(fseg) - fflat);
			fflat = std::begin(++fseg);
		}
		flat::destruct_n(fflat, lflat - fflat);
	}
}

template<typename C>
// C models SegmentedCoordinate
inline
void destruct(C first, C last) {
	return seg::destruct(segment(first), flat(first), segment(last), flat(last));
}


template<typename I0, typename I1, typename P>
// I0 models SegmentIterator
// I1 models ForwardIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal_seg_flat(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1, P p) {
	while (fseg0 != lseg0) {
		std::tie(fflat0, first1) = flat::equal(fflat0, std::end(fseg0), first1, p);
		if (fflat0 != std::end(fseg0)) 
			return false;
		fflat0 = std::begin(++fseg0);
	}
	return flat::equal(fflat0, std::end(fseg0), first1, p);
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models ForwardIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// IteratorValueType<I0> models Regular
bool equal_seg_flat(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 first1) {
	return seg::equal_seg_flat(fseg0, fflat0, lseg0, lflat0, first1, std::equal_to<>());
}

template<typename C, typename I, typename P>
// C models SegmentedCoordinate
// I models InputIterator
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<C> == IteratorValueType<I>
bool equal_seg_flat(C first0, C last0, I first1, P p) {
	return seg::equal_seg_flat(segment(first0), flat(first0), segment(last0), flat(last0), p);
}

template<typename C, typename I>
// C models SegmentedCoordinate
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
bool equal_flat_n_seg(I0 first0, N n0, I1 fseg1, FlatIterator<I1> fflat1, I1 lseg1, FlatIterator<I1> lflat1, P p) {
	while(fseg1 != lseg1) {
		N n1 = static_cast<N>(std::end(fseg1) - fflat1);
		if (n0 < n1 || !flat::equal_n(first0, n1, fflat1))
			return false;
		n0 = n0 - n1;
		first0 = flat::successor(first0, n1);
		fflat1 = std::begin(++fseg1);
 	}

	N n1 = static_cast<N>(lflat1 - fflat1);
	if (n0 < n1 || n0 > n1)
		return false;
	return flat::equal_n(first0, n0, fflat1, p);
}

template<typename I0, typename N, typename I1>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// IteratorValueType<I0> models Regular
bool equal_flat_n_seg(I0 first0, N n, I1 fseg1, FlatIterator<I1> fflat1, I1 lseg1, FlatIterator<I1> lflat1) {
	return seg::equal_flat_n_seg(first0, n, fseg1, fflat1, lseg1, lflat1, std::equal_to<>());
}

template<typename I, typename N, typename C, typename P>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentedCoordinate
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
bool equal_flat_n_seg(I first0, N n, C first1, C last1, P p) {
	return seg::equal_flat_n_seg(first0, n, segment(first1), flat(first1), segment(last1), flat(last1), p);
}

template<typename I, typename N, typename C>
// I0 models ForwardIterator
// N models Integer
// I1 models SegmentedCoordinate
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
bool equal(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 fseg1, FlatIterator<I1> fflat1, P p) {
	
	using Flat0 = FlatIterator<I0>;
	using Diff0 = IteratorDifferenceType<Flat0>;

	while (fseg0 != lseg0) {
		Diff0 n0 = std::end(fseg0) - fflat0;
		Diff0 n1 = static_cast<Diff0>(std::end(fseg1) - fflat1);
		if (n0 < n1) {
			if (!flat::equal_n(fflat0, n0, fflat1, p))
				return false;
			fflat0 = std::begin(++fseg0);
			fflat1 = flat::successor(fflat1, n0);
		}
		if (!flat::equal_n(fflat0, n1, fflat1, p))
			return false;

		fflat1 = std::begin(++fseg1);
		if (n0 > n1)
			fflat0 = flat::successor(fflat0, n1);
		else
			fflat0 = std::begin(++fseg0);
	}
	return seg::equal_flat_n_seg(fflat0, lflat0 - fflat0, fseg1, fflat1, p);
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
bool equal(I0 fseg0, FlatIterator<I0> fflat0, I0 lseg0, FlatIterator<I0> lflat0, I1 fseg1, FlatIterator<I1> fflat1) {
	return seg::equal(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, std::equal_to<>());
}

template<typename C0, typename C1, typename P>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
// P models EquivalenceRelation
// Domain<P> == IteratorValueType<C0> == IteratorValueType<C1>
bool equal(C0 first0, C0 last0, C1 first1, P p) {
	return seg::equal(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), p);
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
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
int lexicographical_compare_flat_n_seg(I0 first0, N n0, I1 fseg1, I1 fflat1, I1 lseg1, I1 lflat1, Cmp cmp) {
	while (fseg1 != lseg1) {
		N n1 = static_cast<N>(std::end(fseg1) - fflat1);
		int r = flat::compare_n(first0, std::min(n0, n1), fflat1, cmp);
		if (r != 0) return r;
		if (n0 <= n1) return -1;
		n0 = n0 - n1;
		first0 = flat::successor(first0, n1);
		fflat1 = std::begin(++fseg1);
	}
	N n1 = static_cast<N>(lflat1 - fflat1);
	int r = flat::compare_n(first0, std::min(n0, n1), fflat1, cmp);
	if (r != 0) return r;
	if (n0 <= n1) return -1;
	return 1;
}

template<typename I0, typename I1, typename Cmp>
// I0 models SegmentIterator
// I1 models SegmentIterator
// Cmp models StrictWeakOrdering
// Domain<P> == IteratorValueType<I0> == IteratorValueType<I1>
int lexicographical_compare(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1,
	I1 lseg1,
	FlatIterator<I1> lflat1,
	Cmp cmp) {

	using Flat0 = FlatIterator<I0>;
	using Diff0 = IteratorDifferenceType<Flat0>;
	using Flat1 = FlatIterator<I1>;
	using Diff1 = IteratorDifferenceType<Flat1>;

	while (fseg0 != lseg0 && fseg1 != lseg0) {
		Diff0 n0 = std::end(fseg0) - fflat0;
		Diff0 n1 = static_cast<Diff0>(std::end(fseg1) - fflat1);
		int r = flat::equal_n(fflat0, std::min(n0, n1), fflat1);
		if (r != 0)
			return r;
		if (n0 < n1) {
			fflat0 = std::begin(++fseg0);
			fflat1 = flat::successor(fflat1, n0);
		}
		fflat1 = std::begin(++fseg1);
		if (n0 > n1)
			fflat0 = flat::successor(fflat0, n1);
		else
			fflat0 = std::begin(++fseg0);
	}

	if (fseg0 == lseg0) {
		Diff0 n0 = lflat0 - fflat0;
		if (fseg1 == lseg1) {
			Diff0 n1 = static_cast<Diff0>(lflat1 - fflat1);
			int r = flat::lexicographical_compare_n(fflat0, std::min(n0, n1), fflat1, cmp);
			return r == 0 ? n0 < n1 : r;
		}
		return seg::lexicographical_compare_flat_n_seg(fflat0, n0, fseg1, fflat1, lseg1, lflat1, cmp);
	}
	else {
		return -seg::lexicographical_compare_flat_n_seg(fflat1, lflat1 - fflat1, fseg0, fflat0, lseg0, lflat0, cmp);
	}
}

template<typename I0, typename I1>
// I0 models SegmentIterator
// I1 models SegmentIterator
// IteratorValueType<I0> == IteratorValueType<I1>
int lexicographical_compare(
	I0 fseg0,
	FlatIterator<I0> fflat0,
	I0 lseg0,
	FlatIterator<I0> lflat0,
	I1 fseg1,
	FlatIterator<I1> fflat1,
	I1 lseg1,
	FlatIterator<I1> lflat1) {
	return seg::lexicographical_compare(fseg0, fflat0, lseg0, lflat0, fseg1, fflat1, lseg1, lflat1, std::less<>());
}

template<typename C0, typename C1, typename Cmp>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
// Cmp models StrictWeakOrdering
// Domain<Cmp> == IteratorValueType<I0> == IteratorValueType<I1>
int lexicographical_compare(
	C0 first0, C0 last0, C1 first1, C1 last1, Cmp cmp) {
	return seg::lexicographical_compare(segment(first0), flat(first0), segment(last0), flat(last0), segment(first1), flat(first1), segment(last1), flat(last1), cmp);
}

template<typename C0, typename C1>
// C0 models SegmentedCoordinate
// C1 models SegmentedCoordinate
// IteratorValueType<I0> == IteratorValueType<I1>
int lexicographical_compare(
	C0 first0, C0 last0, C1 first1, C1 last1) {
	return seg::lexicographical_compare(first0, last0, first1, last1, std::less<>());
}




template<typename I>
// I models SegmentIterator
std::size_t distance(I fseg, FlatIterator<I> fflat, I lseg, FlatIterator<I> lflat) {
	std::size_t d = 0;
	while (fseg != lseg) {
		d = d + static_cast<std::size_t>(std::end(fseg) - fflat);
		fflat = std::begin(++fseg);
	}
	return d + static_cast<std::size_t>(lflat - fflat);
}

template<typename C>
// C models SegmentedCoordinate
IteratorDifferenceType<C> distance(C first, C last) {
	return static_cast<IteratorDifferenceType<C>>(seg::distance(segment(first), flat(first), segment(last), flat(last)));
}


template<typename I, typename N>
// I models SegmentIterator
// N models Integer
inline
std::pair<I, FlatIterator<I>> successor(I fseg, FlatIterator<I> fflat, N n) {
	using DiffType = std::ptrdiff_t;

	DiffType _n = static_cast<DiffType>(n);
	FlatIterator<I> lflat = std::end(fseg);
	DiffType d = lflat - fflat;
	while (_n >= d && _n > 0) {
		_n = _n - d;
		fflat = std::begin(++fseg);
		d = std::end(fseg) - fflat;
	}
	return { fseg, flat::successor(fflat, _n) };
}

template<typename C, typename N>
// C models SegmentedCoordinate
// N models Integer
inline
C successor(C first, N n) {
	return C(seg::successor(segment(first), flat(first), n));
}

template<typename I, typename N>
// I models SegmentIterator
// N models Integer
inline
std::pair<I, FlatIterator<I>> predecessor(I lseg, FlatIterator<I> lflat, N n) {
	using DiffType = std::ptrdiff_t;

	DiffType _n = static_cast<DiffType>(n);
	FlatIterator<I> fflat = std::begin(lseg);
	DiffType d = lflat - fflat;
	while (_n > d) {
		_n = _n - d;
		lflat = std::end(--lflat);
		d = lflat - std::begin(lflat);
	}
	return { lflat,  flat::predecessor(lflat, _n) };
}

template<typename C, typename N>
// C models SegmentedCoordinate
// N models Integer
inline
C predecessor(C first, N n) {
	return C(seg::predecessor(segment(first), flat(first), n));
}

} // namespace seg

} // namespace str2d
