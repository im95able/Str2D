#pragma once

#include <tuple>
#include <algorithm>

#include "flat_algorithm.h"
#include "seg_container_base.h"
#include "utility.h"

namespace str2d
{ 

namespace seg 
{

//************************************************************************
// FLAT INSERT
//************************************************************************
// Code for flat insertion in accordance to the balance rule of the 
// segmented insertion. That's why this code is here and not in some "flat..." file.

// Moves "[first, insert)" "n" positions forward or "[insert, last) "n" positions backward
// depending on which of those two ranges is smaller.
template<typename I, typename N, bool U>
// I models RandomAccessIterator
// N models Integer
std::pair<I, I> _insert_flat_front_back_available(I firstu, I lastu, I first, I last, I insert, N n) {
	// precondition: [firstu, first) is a valid range
	// precondition: [last, lastu) is a valid range
	// precondition: first - firstu >= n
	// precondition: lastu - last >= n
	// precondition: insert belongs to [first, last]
	IteratorDifferenceType<I> left = insert - first;
	IteratorDifferenceType<I> right = last - insert;
	if (left < right || (left == right && (first - firstu < lastu - last))) {
		if constexpr (U) flat::slide_cut_n(first, left, static_cast<IteratorDifferenceType<I>>(n));
		else			 flat::move_n(first, left, flat::predecessor(first, n));
		return { flat::predecessor(first, n), last };
	}
	else {
		if constexpr (U) flat::slide_cut_backward_n(last, right, static_cast<IteratorDifferenceType<I>>(n));
		else			 flat::move_backward_n(last, right, flat::successor(last, n));
		return { first, flat::successor(last, n) };
	}
}

// Moves both "[first, insert)" forward and "[insert, last)" backward.
// The new range is centered in the range "[firstu, lastu)".
template<typename I, typename N, bool U>
// I models RandomAccessIterator
// N models Integer
std::pair<I, I> _insert_flat_front_back_available_combined(I firstu, I lastu, I first, I last, I insert, N n) {
	// precondition: [firstu, first) is a valid range
	// precondition: [last, lastu) is a valid range
	// precondition: insert belongs to [first, last]
	// precondition: n > first - firstu 
	// precondition: n > lastu - last
	// precondition: n <= (first - firstu) + (lastu - last)
	IteratorDifferenceType<I> _n = static_cast<IteratorDifferenceType<I>>(n);
	IteratorDifferenceType<I> front = first - firstu;
	IteratorDifferenceType<I> back = lastu - last;
	IteratorDifferenceType<I> f = (front + back - _n) >> 1;
	front = front - f;
	back = _n - front;
	if constexpr (U) {
		flat::slide_cut_n(first, insert - first, front);
		flat::slide_cut_backward_n(last, last - insert, back);
	}
	else {
		flat::move_n(first, insert - first, first - front);
		flat::move_backward_n(last, last - insert, last + back);
	}
	return { first - front, last + back };
}


// Moves "[first, insert)" forward and/or "[insert, last)" backward in such a way
// that after the move the following postconditions are satisfied :
// postcondition: [return.first, return.first + (insert - first)) is equal to [first, insert)
// postcondition: [return.second - (last - insert), return.second) is equal to [insert, last)
// postcondition: distance(return.first + (insert - first), return.second - (last - insert)) is equal to "n" 
template<typename I, typename N, bool U>
// I models RandomAccessIterator
// N models Integer
inline
std::pair<I, I> _insert_flat(I firstu, I lastu, I first, I last, I insert, N n) {
	// precondition: [firstu, first) is a valid range
	// precondition: [last, lastu) is a valid range
	// precondition: insert belongs to [first, last]
	// precondition: n <= (first - firstu) + (lastu - last)
	IteratorDifferenceType<I> _n = static_cast<IteratorDifferenceType<I>>(n);
	if (first - firstu >= _n) {
		if (lastu - last >= _n) 
			return _insert_flat_front_back_available<I, N, U>(firstu, lastu, first, last, insert, _n);

		if constexpr (U) flat::slide_cut_n(first, insert - first, _n);
		else			 flat::move_n(first, insert - first, flat::predecessor(first, _n));
		return { flat::predecessor(first, _n), last };
	}
	if (lastu - last >= _n) {
		if constexpr (U) flat::slide_cut_backward_n(last, last - insert, _n);
		else			 flat::move_backward_n(last, last - insert, flat::successor(last, _n));
		return { first, flat::successor(last, _n) };
	}
	return _insert_flat_front_back_available_combined<I, N, U>(firstu, lastu, first, last, insert, _n);
}

// Same as "_insert_flat". 
// Pressuposses that "[firstu, first)" and "[last, lastu)" are initialized
template<typename I, typename N>
// I models RandomAccessIterator
// N models Integer
inline
std::pair<I, I> insert_flat(I firstu, I lastu, I first, I last, I insert, N n) {
	// precondition: [firstu, first) is a valid range
	// precondition: [last, lastu) is a valid range
	// precondition: insert belongs to [first, last]
	// precondition: n <= (first - firstu) + (lastu - last)
	return _insert_flat<I, N, false>(firstu, lastu, first, last, insert, n);
}

// Same as "_insert_flat". 
// Pressuposses that "[firstu, first)" and "[last, lastu)" are uninitialized
template<typename I, typename N, bool U = true>
// I models RandomAccessIterator
// N models Integer
inline
std::pair<I, I> insert_flat_uninitialized(I firstu, I lastu, I first, I last, I insert, N n) {
	// precondition: [firstu, first) is a valid uninitialized range
	// precondition: [last, lastu) is a valid uninitialized range
	// precondition: insert belongs to [first, last]
	// precondition: n <= (first - firstu) + (lastu - last)
	return _insert_flat<I, N, true>(firstu, lastu, first, last, insert, n);
}
//************************************************************************
// ~FLAT INSERT
//************************************************************************



//************************************************************************
// FLAT ERASE
//************************************************************************
// Code for flat erasure is in the file for segmented code for the same reason as
// "FLAT INSERT" code

// Erases the range "[firste, laste)" by either moving 
// "[first, firste)" "laste - firste" positions backward or
// "[laste, last)" "laste - firste" positions forward depending on what will the 
// less moves
template<typename I, bool U>
// I models RandomAccessIterator
inline
std::tuple<I, I, I> _erase_flat(I first, I last, I firste, I laste) {
	// precondition: [firstu, lastu) is a valid range
	// precondition: [first, last] belongs to [firstu, lastu]
	// precondition: [firste, laste] belongs to [first, last]
	IteratorDifferenceType<I> n = laste - firste;
	if (firste - first < last - laste) {
		flat::move_backward_n(firste, firste - first, laste);
		if constexpr (U) 
			flat::destruct_n(first, laste - firste);
		return { first + n, last, laste };
	}
	else {
		flat::move_n(laste, last - laste, firste);
		if constexpr (U) 
			flat::destruct_backward_n(last, laste - firste);
		return { first, last - n, firste };
	}
}

// Same as "_erase_flat"
template<typename I>
// I models RandomAccessIterator
inline
std::tuple<I, I, I> erase_flat(I first, I last, I firste, I laste) {
	// precondition: [firstu, lastu) is a valid range
	// precondition: [first, last] belongs to [firstu, lastu]
	// precondition: [firste, laste] belongs to [first, last]
	return _erase_flat<I, false>(first, last, firste, laste);
}

// Same as "_erase_flat"
template<typename I>
// I models RandomAccessIterator
inline
std::tuple<I, I, I> erase_flat_uninitialized(I first, I last, I firste, I laste) {
	// precondition: [firstu, lastu) is a valid range
	// precondition: [first, last] belongs to [firstu, lastu]
	// precondition: [firste, laste] belongs to [first, last]
	return _erase_flat<I, true>(first, last, firste, laste);
}

//************************************************************************
// ~FLAT ERASE
//************************************************************************



//************************************************************************
// SLIDE
//************************************************************************

// Calculates the "begin index" for the given capacity("c") and size("s") 
inline
size_t balanced_begin(size_t c, size_t s) {
	return (c - s) >> 1;
}

// Calculates the "end index" for the given capacity("c") and size("s") 
inline
size_t balanced_end(size_t c, size_t s) {
	return balanced_begin(c, s) + s;
}

// Moves the "segment" "n" positions forward.
// It's unguarded in a sense that it doesn't check if the "back available range" is large enough to take
// "n" elements.
template<typename H>
// H models SegmentHeader
inline
void slide_segment(H& h, size_t n) {
	flat::slide_cut_n(seg::begin(h), seg::size(h), n);
	decrease_begin_index(h, n);
	decrease_end_index(h, n);
}

// Moves the "segment" "n" positions backward.
// It's unguarded in a sense that it doesn't check if the "front available range" is large enough to take
// "n" elements.
template<typename H>
// H models SegmentHeader
inline
void slide_segment_backward(H& h, size_t n) {
	flat::slide_cut_backward_n(seg::end(h), seg::size(h), n);
	increase_begin_index(h, n);
	increase_end_index(h, n);
}
//************************************************************************
// ~SLIDE
//************************************************************************


//************************************************************************
// MOVE
//************************************************************************

// Does a series of concatenating moves from the front of the "curr segment" to the front of "left back range".
// Moves "n0" elements, skips "e" elements, and then moves another "n1" elements.
// Doesn't check whether the "left back range" is large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_left_back_available(
	H& curr, H& left, size_t n0, size_t e, size_t n1) {
	// precondition: back(left) >= n0 + e + n1
	// precondition: size(curr) >= n0 + n1

	flat::move_n_uninitialized(seg::begin(curr), n0, seg::end(left));
	flat::move_n_uninitialized(flat::successor(seg::begin(curr), n0), n1, flat::successor(seg::end(left), n0 + e));
	flat::destruct_n(seg::begin(curr), n0 + n1);
	increase_end_index(left, n0 + e + n1);
	increase_begin_index(curr, n0 + n1);
}

// Moves "n0" elements from the front of the "curr segment" to the front of the "left back range".
// Doesn't check whether the "left back range" is large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_left_back_available(
	H& curr, H& left, size_t n) {
	// precondition: back(left) >= n
	// precondition: size(curr) >= n

	flat::move_n_uninitialized(seg::begin(curr), n, seg::end(left));
	flat::destruct_n(seg::begin(curr), n);
	increase_end_index(left, n);
	increase_begin_index(curr, n);
}

// Does a series of concatenating moves from the front of the "curr segment" to the front of "left back range".
// Moves "n0" elements, skips "e" elements, and then moves another "n1" elements.
// If "left back range" is not large enough to hold all new elements the "left segment" is moved forward in order to make space.
// Available space of "left" must be large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_left(
	H& curr, H& left, size_t n0, size_t e, size_t n1) {
	// precondition: available(left) >= n0 + e + n1
	// precondition: size(curr) >= n0 + n1

	size_t n = n0 + e + n1;
	if (back(left) < n) 
		slide_segment(left, begin_index(left) - balanced_begin(capacity(left), seg::size(left) + n));
	move_to_left_back_available(curr, left, n0, e, n1);
}

// Moves "n0" elements from the front of the "curr segment" to the front of the "left back range".
// If "left back range" is not large enough to hold all new elements the "left segment" is moved forward in order to make space.
// Available space of "left" must be large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_left(
	H& curr, H& left, size_t n) {
	// precondition: available(left) >= n
	// precondition: size(curr) >= n

	if (back(left) < n)
		slide_segment(left, begin_index(left) - balanced_begin(capacity(left), seg::size(left) + n));
	move_to_left_back_available(curr, left, n);
}


// Does a series of backward concatenating moves from the back of the "curr segment" to the back of "right front range".
// Moves "n0" elements, skips "e" elements, and then moves another "n1" elements.
// Doesn't check whether the "right front range" is large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_right_front_available(
	H& curr, H& right, size_t n0, size_t e, size_t n1) {
	// precondition: front(right) >= n0 + e + n1
	// precondition: size(curr) >= n0 + n1

	flat::move_backward_n_uninitialized(seg::end(curr), n0, seg::begin(right));
	flat::move_backward_n_uninitialized(flat::predecessor(seg::end(curr), n0), n1, flat::predecessor(seg::begin(right), n0 + e));
	flat::destruct_backward_n(seg::end(curr), n0 + n1);
	decrease_begin_index(right, n0 + e + n1);
	decrease_end_index(curr, n0 + n1);
}

// Backward moves "n0" elements from the back of the "curr segment" to the back of the "right front range".
// Doesn't check whether the "right front range" is large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_right_front_available(
	H& curr, H& right, size_t n) {
	// precondition: front(right) >= n
	// precondition: size(curr) >= n

	flat::move_backward_n_uninitialized(seg::end(curr), n, seg::begin(right));
	flat::destruct_backward_n(seg::end(curr), n);
	decrease_begin_index(right, n);
	decrease_end_index(curr, n);
}


// Does a series of backward concatenating moves from the back of the "curr segment" to the back of "right front range".
// Moves "n0" elements, skips "e" elements, and then moves another "n1" elements.
// If "right front range" is not large enough to hold all new elements the "right segment" is moved backward in order to make space.
// Available space of "right" must be large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_right(
	H& curr, H& right, size_t n0, size_t e, size_t n1) {
	// precondition: available(right) >= n0 + e + n1
	// precondition: size(curr) >= n0 + n1
	size_t n = n0 + e + n1;
	if (front(right) < n)
		slide_segment_backward(right, balanced_end(capacity(right), seg::size(right) + n) - end_index(right));
	move_to_right_front_available(curr, right, n0, e, n1);
}

// Backward moves "n0" elements from the back of the "curr segment" to the back of the "right front range".
// If "right front range" is not large enough to hold all new elements the "right segment" is moved backward in order to make space.
// Available space of "right" must be large enough to hold all new elements.
template<typename H>
// I models SegmentHeader
inline
void move_to_right(
	H& curr, H& right, size_t n) {
	// precondition: available(right) >= n
	// precondition: size(curr) >= n
	if (front(right) < n)
		slide_segment_backward(right, balanced_end(capacity(right), seg::size(right) + n) - end_index(right));
	move_to_right_front_available(curr, right, n);
}


// Sets "f" as the first index of "right", and moves "n" elements to that point from the
// back of the "curr segment"
// Doesn't check whether the mvoe will overflow "right data" or is the number of moved elements 
// larger than the size of "curr segment"
template<typename H>
// I models SegmentHeader
inline
void move_to_right_empty(
	H& curr, H& right, size_t f, size_t n) {
	// precondition: f + n <= capacity(*right)
	// precondition: size(right) >= move
	flat::move_n_uninitialized(seg::end(curr) - n, n, seg::data(right) + f);
	flat::destruct_backward_n(seg::end(curr), n);
	set_begin_index(right, f);
	set_end_index(right, f + n);
	decrease_end_index(curr, n);
}
//************************************************************************
// ~MOVE
//************************************************************************



//************************************************************************
// SEGMENTED INSERT
//************************************************************************

// Splits the "curr segment" at the "i"-th index and creates an unitialzed range of "n" elements at the position where split occurred
template<typename H>
// H models SegmentHeader
inline
void insert_current_available(H& curr, size_t i, size_t n) {
	// precondition: i <= size(curr)
	// precondition: n <= available(curr)
	using Iterator = ValueType<H>*;
	Iterator first = seg::begin(curr);
	Iterator last = seg::end(curr);
	std::tie(first, last) = 
		insert_flat_uninitialized(data(curr), data(curr) + capacity(curr), first, last, flat::successor(first, i), n);
	set_begin_index(curr, static_cast<size_t>(first - data(curr)));
	set_end_index(curr, static_cast<size_t>(last - data(curr)));
}

// Splits the "curr segment" at the "i"-th index and creates an unitialzed range of "n" elements at the position where split occurred
template<typename I>
// I models SegmentHeaderIterator
inline
pair2<I, size_t> insert_current_available_aux(I curr, size_t i, size_t n) {
	// precondition: i <= size(*curr)
	// precondition: n <= available(*curr)
	insert_current_available(*curr, i, n);
	return { { curr, i }, { curr, i + n } };
}

// Helper function for calculating size based on current "range mod"
inline
std::pair<size_t, size_t> check_increase_by_mod(size_t m, size_t s) {
	return m > 0 ? std::pair<size_t, size_t>{ m - 1, s + 1 } : std::pair<size_t, size_t>{ m, s };
}

// Helper function for calculating size based on current "range mod"
// Automatically decreasses "m" is it's greater than zero
inline
size_t check_increase_by_mod_aux(size_t& m, size_t s) {
	if (m > 0) {
		--m;
		return s + 1;
	}
	return s;
}

// Sets "begin index" to "balanced_begin(capacity(empty), s)" and "end index" to "balanced_begin(capacity(empty), s) + s"
template<typename H>
// H models SegmentHeader
inline 
void set_segment_bounds(H& empty, size_t s) {
	size_t f = balanced_begin(capacity(empty), s);
	set_begin_index(empty, f);
	set_end_index(empty, f + s);
}

// Sets "begin index" to "balanced_begin(capacity(empty), s)" and "end index" to "balanced_begin(capacity(empty), s) + s" 
// for a range of segments "[first, first + n)"
template<typename I, typename N>
// I models SegmentHeaderIterator
// N models Integer
inline
I set_segment_bounds(I first, N n, size_t s) {
	while (n) {
		set_segment_bounds(*first, s);
		--n;
		++first;
	}
	return first;
}


// Creates a segment range for "n" new elements
template<typename I, typename Proc>
// I models SegmentHeaderIterator
// Proc models Procedure
// Arity<Proc> == 2
// InputType<Proc, 0> == IteratorValueType<I>
// InputType<Proc, 1> == size_t
inline
std::tuple<I, size_t, size_t> create_range(
	I first, size_t m, size_t s, size_t n, Proc p) {
	auto [_m, _s] = check_increase_by_mod(m, s);
	while (_s <= n) {
		p(*first, _s);
		n = n - _s;
		++first;
		m = _m;
		std::tie(_m, _s) = check_increase_by_mod(_m, s);
	}
	return { first, m, n };
}

// Moves "s" elements from the front of "curr" to the back "left"
template<typename H>
// H models SegmentHeader
struct move_to_left_filled {
	H* curr;

	move_to_left_filled(H& curr) : curr(&curr) {}

	void operator()(H& left, size_t s) {
		set_begin_end_indices(left, balanced_begin(capacity(left), s));
		move_to_left_back_available(*curr, left, s);
	}
};

// Sets empty segment bounds for size "s"
template<typename H>
// H models SegmentHeader
struct move_to_left_empty {
	void operator()(H& left, size_t s) {
		set_segment_bounds(left, s);
	}
};

// "Pre" range has been depleted; now we need to deplete "n" range and "post" range of size "size(*curr)".
// All segments in the range [first, curr) are empty; hence the function name has the "empty" suffix.
template<typename I>
// I models SegmentHeaderIterator
inline
std::pair<I, size_t> insert_balance_left_increase_empty(
	I first, I curr, size_t m, size_t s, size_t n) {
	// precondition: for every segment header("h") in the range [first, curr): begin_index(h) == begin_index(h) == "capacity"
	// precondition: 0 <= capacity * (curr - first + 1) - (n + size(*curr)) <= "capacity"
	// precondition: m <= curr - first

	// We deplete "n" empty positions by creating segments which have only empty positions in them.
	std::tie(first, m, n) = create_range(first, m, s, n, move_to_left_empty<IteratorValueType<I>>());
	// After the depletion there will be some number of positions less than the 
	// size of the next "segment".
	// precondition: n < check_increase_by_mod(m, s).second
	if (first == curr - 1) {
		// There are 2 segments left, "first" and "curr".
		// "first" will take both empty positions and filled ones.
		size_t left_size = check_increase_by_mod(m, s).second;
		size_t move = left_size - n;
		set_begin_end_indices(*first, balanced_begin(capacity(*first), left_size));
		move_to_left_back_available(*curr, *first, 0, n, move);
		return { first, seg::size(*first) - move };
	}
	if (first == curr) {
		// "curr" segment is the only available and it has to take "n" empty positions
		return insert_current_available_aux(curr, 0, n).second;
	}
	// precondition: first == curr + 1
	// "curr" has only empty positions; all filled positions of "curr" were moved to segments left of it.
	// This can only happen if the insert pos("i") was equal to the size of "curr".
	return { curr, seg::size(*curr) };
}

// We have "pre", "n", and "post" ranges to deplete. 
// Either "empty(*first)" or "size(*first) < check_increase_by_mod(m, s).second"
// All segments in the range [first, curr) need to be increased in size; hence the name
// of theg function has the "increase" suffix
// - size of the "pre" range == i
// - size of the "n" range == n
// - size of the "post" range == size(*curr) - i
template<typename I>
// I models SegmentHeaderIterator
inline
pair2<I, size_t> insert_balance_left_increase(
	I first, I curr, size_t m, size_t s, size_t i, size_t n) {
	// precondition: curr - first > 0
	// precondition: seg::size(*first) < check_increase_by_mod(m, s).second
	// precondition: for every segment header("h") in the range [first + 1, curr): begin_index(h) == end_index(h) == "capacity"
	// precondition: 0 <= capacity * (curr - first + 1) - (seg::size(*first) + n + seg::size(*curr)) < "capacity"
	// precondition: m <= curr - first

	size_t move = check_increase_by_mod_aux(m, s) - seg::size(*first);
	if (move <= i) {
		move_to_left(*curr, *first, move);
		std::tie(first, m, i) = create_range(first + 1, m, s, i - move, move_to_left_filled<IteratorValueType<I>>(*curr));
		if (first == curr) {
			// Inserted range is entirely on the "curr" segment
			return insert_current_available_aux(curr, i, n);
		}
		move = check_increase_by_mod_aux(m, s) - seg::size(*first);
	}
	// precondition: move > i
	// precondition: curr - first > 0
	if (move >= i + n) {
		// Inserted range is entirely on the "first" segment
		move = move - (i + n);
		move_to_left(*curr, *first, i, n, move);
		size_t l = seg::size(*first) - move;
		return { { first, l - n }, { first, l } };
	}
	// Beginning of the inserted range is on the "first" segment
	move = move - i;
	move_to_left(*curr, *first, i, move, 0);
	return { { first, seg::size(*first) - move }, insert_balance_left_increase_empty(first + 1, curr, m, s, n - move) };
}

// The only thing which is known at this point is that the minimal number of "areas" have
// been allocated to accommodate all new elements.
// We have "pre", "n", and "post" ranges to deplete.
// - size of the "pre" range == i
// - size of the "n" range == n
// - size of the "post" range == size(*curr) - i
template<typename I>
// I models SegmentHeaderIterator
inline
pair2<I, size_t> insert_balance_left(
	I first, I curr, size_t m, size_t s, size_t i, size_t n) {
	// precondition: curr - first > 0
	// precondition: 0 <= capacity * (curr - first + 1) - (size(*first) + n + size(*curr)) < capacity
	// precondition: m <= curr - first

	auto [_m, new_left_size] = check_increase_by_mod(m, s);
	if (seg::size(*first) >= new_left_size) {
		// Size of the "first" segment(previously the segment to the left of "curr") is larger 
		// than it should be. We move some elements to the empty segment which is to the right of it.
		size_t f = balanced_begin(capacity(*first), check_increase_by_mod(_m, s).second);
		move_to_right_empty(*first, *(first + 1), f, seg::size(*first) - new_left_size);
		++first;
		m = _m;
	}
	return insert_balance_left_increase(first, curr, m, s, i, n);
}

// Available size of "curr" and "right" is large enough to hold all new elements.
// This function will be called  when a single inserted element can't fit on and 
// the left segment is filled to capacity but the right one is not.
template<typename I>
// I models SegmentHeaderIterator
inline
pair2<I, size_t> insert_balance_right_simple(
	I curr, I right, size_t new_curr_size, size_t i, size_t n) {
	// precondition: 2 * "capacity" >= new_curr_size + seg::size(*right) + n

	if (new_curr_size >= i + n) {
		// Inserted range is entirely on "curr".
		move_to_right(*curr, *right, seg::size(*curr) + n - new_curr_size);
		return insert_current_available_aux(curr, i, n);
	}
	if (new_curr_size > i) {
		// Beginning of the inserted range is on "curr", ending is on "right".
		size_t e0 = new_curr_size - i;
		size_t e1 = static_cast<size_t>(n - e0);
		move_to_right(*curr, *right, seg::size(*curr) - i, e1, 0);
		insert_current_available(*curr, i, e0);
		return { { curr, seg::size(*curr) - e0 }, { right, e1 } };
	}
	// Inserted range is entirely on "right".
	size_t f = i - new_curr_size;
	move_to_right(*curr, *right, seg::size(*curr) - i, n, f);
	return { { right, f }, { right, f + n } };
}

// Available size of "curr" and "left" is large enough to hold all new elements
// This function will be called in the vast majority of times, when a single elements gets inserted and 
// the "left" segment is not filled to capacity.
template<typename I>
// I models SegmentHeaderIterator
inline
pair2<I, size_t> insert_balance_left_simple(
	I curr, I left, size_t new_curr_size, size_t i, size_t n) {
	// precondition: 2 * "capacity" >= new_curr_size + seg::size(*left) + n

	size_t backward_i = seg::size(*curr) - i;
	if (new_curr_size >= backward_i + n) {
		// Inserted range is entirely on "curr".
		move_to_left(*curr, *left, seg::size(*curr) + n - new_curr_size);
		return insert_current_available_aux(curr, seg::size(*curr) - backward_i, n);
	}
	if (new_curr_size > backward_i) {
		// Beginning of the inserted range is on "left", ending is on "curr".
		size_t e1 = new_curr_size - backward_i;
		size_t e0 = n - e1;
		move_to_left(*curr, *left, i, e0, 0);
		insert_current_available(*curr, 0, e1);
		return { { left, seg::size(*left) - e0 }, { curr, e1 } };
	}
	// Inserted range is entirely on "left".
	size_t backward_f = backward_i - new_curr_size;
	move_to_left(*curr, *left, i, n, backward_f);
	size_t l = seg::size(*left) - backward_f;
	return { { left, l - n }, { left, l } };
}

// Calculates :
// 1) minimal number of needed segments
// 2) how many of them will be larger than "size" by one
// 3) "size"
std::tuple<size_t, size_t, size_t> segment_range_info(
	size_t capacity, size_t left_size, size_t curr_size, size_t n) {
	size_t s = left_size + curr_size + n;
	std::pair<size_t, size_t> r = division_with_remainder(s, capacity);
	size_t nm_segments = r.second > 0 ? r.first + 1 : r.first;

	r = division_with_remainder(s, nm_segments);
	return { nm_segments, r.second, r.first };
}

// New segments need to be allocated to the left of "curr" and "curr" is the "first segment"
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_left_empty(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// precondition: curr == begin(index)

	auto [nm_segments, m, s] = segment_range_info(capacity(*curr), 0, seg::size(*curr), n);
	Iterator<I> first = insert(index, curr, static_cast<SizeType<I>>(nm_segments - 1));
	return insert_balance_left_increase(first, flat::successor(first, nm_segments - 1), m, s, i, n);
}

// New segments need to be allocated to the left of "curr".
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_left(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// precondition: curr > begin(index)
	// precondition: available(*(curr - 1)) + available(*curr) < n

	auto [nm_segments, m, s] = segment_range_info(capacity(*curr), seg::size(*(curr - 1)), seg::size(*curr), n);
	Iterator<I> first = insert(index, curr, static_cast<SizeType<I>>(nm_segments - 2));
	return insert_balance_left(first - 1, flat::successor(first, nm_segments - 2), m, s, i, n);
}

// There exist segments to both side of "curr"
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_left_right_exist(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// precondition: curr != begin(index)
	// precondition: !empty(*--curr) && !empty(*++curr)
	// precondition: available(*curr) < n

	Iterator<I> left = curr - 1;
	if (available(*curr) + available(*left) >= n)
		return insert_balance_left_simple(curr, left, (size(*curr) + size(*left) + n) >> 1, i, n);
	
	Iterator<I> right = curr + 1;
	if (available(*curr) + available(*right) >= n)
		return insert_balance_right_simple(curr, right, (size(*curr) + size(*right) + n) >> 1, i, n);

	return insert_left(index, curr, i, n);
}

// "curr" is the "last segment" and there exist a segment to the left of "curr"
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_left_exists(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// precondition: curr != begin(index)
	// precondition: !empty(*--curr)
	// precondition: available(*curr) < n

	Iterator<I> left = curr - 1;
	if (available(*curr) + available(*left) >= n) {
		// We can insert all new elements on "curr" and "left"
		return insert_balance_left_simple(curr, left, (seg::size(*curr) + seg::size(*left) + n) >> 1, i, n);
	}
	// New segments need to be allocated to the left of "curr".
	return insert_left(index, curr, i, n);
}

// "curr" is the "first segment" and there exist a segment to the right of "curr"
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_right_exists(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// precondition: curr == begin(index)
	// precondition: !empty(*++curr)
	// precondition: available(*curr) < n

	Iterator<I> right = curr + 1;
	if (available(*curr) + available(*right) >= n) {
		// We first try to balance to to "right"
		return insert_balance_right_simple(curr, right, (seg::size(*curr) + seg::size(*right) + n) >> 1, i, n);
	}
	// New segments need to be allocated(I chose to do the allocations always to the left of "curr").
	return insert_left_empty(index, curr, i, n);
}

// Index is empty.
// We allocate the minimal number of "areas" which are needed to hold all new elements, and 
// create an empty segment range.
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_empty(I& index, size_t n) {
	// precondition: size(index) == 0

	auto[nm_segments, m, s] = segment_range_info(segment_capacity(index), 0, 0, n);
	Iterator<I> first = insert(index, std::begin(index), static_cast<SizeType<I>>(nm_segments));
	set_segment_bounds(first, m, s + 1);
	set_segment_bounds(flat::successor(first, m), nm_segments - m, s);
	Iterator<I> last = flat::successor(first, nm_segments - 1);
	return { { first, size_t(0) }, { last, seg::size(*last) } };
}

// Entry point for insertion
template<typename I>
// I models SegmentIndex
inline
pair2<Iterator<I>, size_t> insert_to_segment_range(
	I& index, Iterator<I> curr, size_t i, size_t n) {
	// Zero new elements are inserted, nothing has to happen
	if (n == 0) return { { curr, i }, { curr, i } }; 

	if (std::size(index) == 0) {
		// There are no allocated areas
		return insert_empty(index, n);
	}
	if (seg::available(*curr) >= n) {
		// "curr" segment can take "n" new elements
		return insert_current_available_aux(curr, i, n);
	}
	if (curr != std::begin(index)) {
		// There exist at least 1 segment to the left of "curr"
		if (curr + 1 != std::end(index)) {
			// There exist at least 1 segment to the left and at least 1 to the right of "curr"
			return insert_left_right_exist(index, curr, i, n);
		}
		return insert_left_exists(index, curr, i, n);
	}
	if (curr + 1 != std::end(index)) {
		// There exist at least 1 segment to the right of "curr"
		return insert_right_exists(index, curr, i, n);
	}
	// There are no segments on either side of "curr"
	return insert_left_empty(index, curr, i, n);
}

//************************************************************************
// ~SEGMENTED INSERT
//************************************************************************




//************************************************************************
// ~SEGMENTED ERASE
//************************************************************************

template<typename I>
// I models SegmentHeaderIterator
inline
size_t destruct_segment_range(I first, I last) {
	size_t s = 0u;
	while (first != last) {
		size_t _s = seg::size(*first);
		flat::destruct_n(seg::begin(*first), _s);
		s = s + _s;
		++first;
	}
	return s;
}

// Erases "n" elements at the "i"-th position of "segment". 
template<typename H>
// H models SegmentHeader
inline
void erase_current(H& curr, size_t i, size_t n) {
	// precondition: i + n <= size(curr)

	ValueType<H>* first = seg::begin(curr);
	ValueType<H>* last = seg::end(curr);
	ValueType<H>* firste = flat::successor(first, i);
	std::tie(first, last, firste) = erase_flat_uninitialized(first, last, firste, flat::successor(firste, n));
	set_begin_index(curr, static_cast<size_t>(first - data(curr)));
	set_end_index(curr, static_cast<size_t>(last - data(curr)));
}

// Only the "curr" segment has decreased in size. We balance if needed with other segments.
template<typename I>
// I models SegmentIndex
inline
std::pair<Iterator<I>, size_t> erase_balance_current(I& index, Iterator<I> curr, size_t i) {
	// precondition: curr != std::end(index)

	size_t s = seg::size(*curr);
	if (s >= limit(*curr)) {
		// "curr" holds over "limit" elements; no balancing needs to happen.
		return { curr, i };
	}

	if (curr == std::begin(index)) {
		// "curr" is the "first segment" and may hold any number of elements greater than 0
		if (s > 0)
			return { curr, i }; // "curr" is not empty or the entire segmented range is empty; no balancing needs to happen.
		// "curr" is empty so it gets erased.
		curr = erase(index, curr);
		return { curr, 0 };
	}

	Iterator<I> left = curr - 1;
	if (available(*left) >= s) {
		// "left" can take all elements of "curr".
		move_to_left(*curr, *left, s);
		left = erase(index, curr) - 1;
		return { left, size(*left) - (s - i) };
	}
	// Elements are balanced equally on "left" and "curr".
	size_t move = ((seg::size(*left) + s) >> 1) - s;
	move_to_right(*left, *curr, move);
	return { curr, move + i };
}


// Balances elements sa that the sizes of "left" and "right" differ at most by one.
template<typename I>
// I models SegmentHeaderIterator
inline
std::pair<I, size_t> erase_balance_left_right_equally(I left, I right) {
	// precondition: n > capacity(*left)

	size_t left_size = seg::size(*left);
	size_t right_size = seg::size(*right);
	size_t h = (left_size + right_size) >> 1;
	if (left_size < h) {
		move_to_left(*right, *left, h - left_size);
		return { left, seg::size(*left) - (h - left_size) };
	}
	else {
		move_to_right(*left, *right, left_size - h);
		return { right, left_size - h };
	}
}

// Both "left" and "right" have decreased in size. We balance them with one another, or with
// the segment to the left of "left".
template<typename I>
// I models SegmentIndex
inline
std::pair<Iterator<I>, size_t> erase_balance_left_right(I& index, Iterator<I> left, Iterator<I> right) {
	size_t left_size = seg::size(*left);
	size_t right_size = seg::size(*right);
	size_t n = left_size + right_size;
	if (n > capacity(*left)) {
		// Remaining elements can't fit on a single segment
		left = erase(index, left + 1, right) - 1;
		return erase_balance_left_right_equally(left, left + 1);
	}
	if (n >= limit(*left) || (left == std::begin(index) && n > 0)) {
		// Remaining elements can fit on a single segment
		move_to_left(*right, *left, right_size);
		left = erase(index, left + 1, right + 1) - 1;
		return { left, left_size };
	}
	if (n == 0) {
		// Zero elements remain on both "left" and "right"; we erase them alongside all segments in between
		return { erase(index, left, right + 1), 0 };
	}
	// Remaining elements are to few to be able to reside on a single segment
	Iterator<I> _left = left - 1;
	if (available(*_left) >= n) {
		// There is enough space on the segment to the left of "left" to take
		// the remaining elements.
		move_to_left(*left, *_left, left_size);
		move_to_left(*right, *_left, right_size);
		_left = erase(index, left, right + 1) - 1;
		return { _left, seg::size(*_left) - right_size };
	}
	// There isn't enough space on the segment to the left of "left" to take the
	// remaning elements.
	move_to_left(*right, *left, right_size);
	size_t _left_size = seg::size(*_left);
	move_to_right(*_left, *left, _left_size - ((_left_size + n ) >> 1));
	left = erase(index, left + 1, right + 1) - 1;
	return { left, seg::size(*left) - right_size };
}


// Entry point for erasure.
template<typename I>
// I models SegmentIndex
inline
std::tuple<Iterator<I>, size_t, size_t> erase_from_segment_range(
	I& index, Iterator<I> left, size_t left_i, Iterator<I> right, size_t right_i) {
	// precondition: std::size(index) > 0 

	if (right == left) {
		// Beginning and ending of the erased range belong to the same "segment".
		if (right_i == left_i)
			return { left, left_i, 0u }; // We are trying to erase zero elements; nothing has to happen
		erase_current(*left, left_i, right_i - left_i);
		auto [it, i] = erase_balance_current(index, left, left_i);
		return { it, i, right_i - left_i };
	}
	size_t s = destruct_segment_range(left + 1, right);
	size_t ls = seg::size(*left) - left_i;
	erase_current(*left, left_i, ls);
	erase_current(*right, 0, right_i);
	auto [it, i] = erase_balance_left_right(index, left, right);
	return { it, i, s + ls + right_i };
}

//************************************************************************
// ~SEGMENTED ERASE
//************************************************************************





//************************************************************************
// INDEX 
//************************************************************************

template<typename C>
// C models SegmentHeaderContainer
inline
void insert_headers(
	C& index, Iterator<C>& used_first, Iterator<C>& used_last, Iterator<C>& insert, SizeType<C> n) {

	using I = Iterator<C>;
	SizeType<C> used_size = static_cast<SizeType<C>>(used_last - used_first);
	if (std::size(index) - used_size >= n) {
		IteratorDifferenceType<I> insert_at = insert - used_first;
		std::tie(used_first, used_last) = insert_flat(
			std::begin(index),
			std::end(index),
			used_first,
			used_last,
			insert,
			n);
		insert = used_first + insert_at;
	}
	else {
		SizeType<C> new_used_size = used_size + n;
		C new_index(new_used_size << 1);
		Iterator<C> new_used_first = flat::successor(std::begin(new_index), new_used_size >> 1);
		IteratorDifferenceType<I> pre_size = insert - used_first;
		insert = flat::move_n(used_first, pre_size, new_used_first).second;
		used_last = flat::move_n(used_first + pre_size, used_size - pre_size, flat::successor(insert, n)).second;
		used_first = new_used_first;
		index = std::move(new_index);
	}
}

template<typename C, typename A>
// A models Allocator
// C models SegmentHeaderContainer
// ValueType<A> == AreaType<IteratorValueType<C>>
inline
void insert_headers_and_allocate_areas(
	C& index, Iterator<C>& used_first, Iterator<C>& used_last, A& alloc, Iterator<C>& insert, SizeType<C> n) {

	using SegmentHeader = IteratorValueType<Iterator<C>>;
	insert_headers(index, used_first, used_last, insert, n);
	Iterator<C> first = insert;
	SizeType<C> _n = n;
	try {
		size_t c = capacity(*first);
		while (_n) {
			set_area(*first, alloc.allocate(1));
			set_begin_end_indices(*first, c);
			++first;
			--_n;
		}
	}
	catch (...) {
		_n = n - _n;
		first = insert;
		while (_n) {
			alloc.deallocate(area(*first), 1);
			--n;
			++first;
		}
		Iterator<C> insert_end = insert + n;
		flat::move_backward(used_first, insert, insert_end);
		used_first = insert_end - (insert - used_first);
		throw;
	}
}


template<typename A>
// A models Allocator
struct deallocate_area {
	A* alloc;
	deallocate_area(A& alloc) : alloc(&alloc) {}

	template<typename H>
	// H models SegmentHeader
	// ValueType<H> == ValueType<A>
	void operator()(H& h) {
		set_begin_end_indices(h, capacity(h));
		alloc->deallocate(area(h), 1);
		set_area(h, nullptr);
	}
};

template<typename I, typename A>
// A models Allocator
// I models SegmentHeaderIterator
// ValueType<A> == AreaType<IteratorValueType<I>>
inline
void erase_headers_and_deallocate_areas(
	I& used_first, I& used_last, A& alloc, I& firste, I laste) {
	std::for_each(firste, laste, deallocate_area<A>(alloc));
	std::tie(used_first, used_last, firste) = erase_flat(used_first, used_last, firste, laste);
}

template<typename C>
// C models SegmentHeaderContainer
inline
std::pair<Iterator<C>, Iterator<C>> middle_edges(C& index) {
	Iterator<C> edge_left = flat::successor(index.begin(), (index.size() >> 1) - 1);
	return { edge_left, flat::successor(edge_left, 1) };
}

// Data structure responsible for holding all "segment headers" and allocating and deallocating
// "segment areas".
template<typename T, std::size_t C, typename A>
// T models
// A models Allocator
class big_header_index
{
public:
	using header_type = big_segment_header<T, C>;
	using value_type = ValueType<header_type>;
	using area_type = AreaType<header_type>;
	using container = std::vector<header_type>;
	using iterator = Iterator<container>;
	using const_iterator = ConstIterator<container>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using size_type = SizeType<container>;
	using allocator = AllocatorRebindType<A, area_type>;
	constexpr static size_t segment_capacity = header_type::capacity;

	container headers;
	iterator edge_left;
	iterator edge_right;
	allocator alloc;

	void _move_from(big_header_index& other) {
		edge_left = other.edge_left;
		edge_right = other.edge_right;
		other.headers = container();
		other.edge_left = other.headers.begin();
		other.edge_right = other.edge_left;
	}

	void move_from(big_header_index&& other) {
		headers = std::move(other.headers);
		alloc = std::move(other.alloc);
		_move_from(other);
	}

	void destroy() {
		if(edge_left != edge_right)
			std::for_each(edge_left, edge_right - 1, deallocate_area<allocator>(alloc));
	}

	void _init() {
		std::tie(edge_left, edge_right) = middle_edges(headers);
		set_area(*edge_left, nullptr);
		set_begin_end_indices(*edge_left, capacity(*edge_left));
	}

	void init() {
		headers.resize(8);
		_init();
	}

public:
	big_header_index(const allocator& alloc = allocator()) : alloc(alloc) { init(); }
	big_header_index(allocator&& alloc) : alloc(std::move(alloc)) { init(); }
	big_header_index(big_header_index&& other) :
		headers(std::move(other.headers)),
		alloc(std::move(other.alloc))
	{
		_move_from(other);
	}
	big_header_index(const big_header_index& other) :
		headers(other.size() + (other.size() >> 1)),
		alloc(other.alloc)
	{
		_init();
	}
	~big_header_index() { destroy(); }

	big_header_index& operator=(big_header_index&& other) {
		destroy();
		move_from(std::move(other));
		return *this;
	}
	big_header_index& operator=(const big_header_index& other) {
		destroy();
		headers.resize(other.size() >> 1);
		alloc = other.alloc();
		_init();
		return *this;
	};

	iterator insert(iterator it, size_type n) {
		// precondition: it belongs to [begin(), end()]
		insert_headers_and_allocate_areas(headers, edge_left, edge_right, alloc, it, n);
		return it;
	}

	iterator erase(iterator first, iterator last) {
		// precondition: [first, last] belongs to [begin(), end()]
		erase_headers_and_deallocate_areas(edge_left, edge_right, alloc, first, last);
		return first;
	}

	void clear() {
		erase(begin(), end());
	}

	friend
	iterator insert(big_header_index& i, iterator it, size_type n) {
		return i.insert(it, n);
	}

	friend
	iterator erase(big_header_index& i, iterator first, iterator last) {
		return i.erase(first, last);
	}

	friend
	iterator insert(big_header_index& i, iterator it) {
		return i.insert(it, 1);
	}

	friend
	iterator erase(big_header_index& i, iterator it) {
		return i.erase(it, it + 1);
	}

	iterator begin() { return edge_left; }
	const_iterator cbegin() const { return const_iterator(edge_left); }
	const_iterator begin() const { return cbegin(); }

	iterator end() { return edge_right - 1; }
	const_iterator cend() const { return const_iterator(edge_right - 1); }
	const_iterator end() const { return cend(); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
	const_reverse_iterator rbegin() const { return crbegin(); }

	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
	const_reverse_iterator rend() const { return crend(); }

	size_t size() const { return static_cast<size_t>(end() - begin()); }
	bool empty() const { return cbegin() == cend(); }
};

// Data structure responsible for holding all "segment headers" and allocating and deallocating
// "segment areas".
template<typename T, std::size_t C, typename A>
// T models
// A models Allocator
class small_header_index
{
public:
	using header_type = small_segment_header<T, C>;
	using value_type = ValueType<header_type>;
	using area_type = AreaType<header_type>;
	using container = std::vector<header_type>;
	using iterator = Iterator<container>;
	using const_iterator = ConstIterator<container>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using size_type = SizeType<container>;
	using allocator = AllocatorRebindType<A, area_type>;
	constexpr static size_t segment_capacity = header_type::capacity;

	container headers;
	iterator edge_left;
	iterator edge_right;
	allocator alloc;

	void _move_from(small_header_index& other) {
		edge_left = other.edge_left;
		edge_right = other.edge_right;
		other.headers = container();
		other.edge_left = other.headers.begin();
		other.edge_right = other.edge_left;
	}

	void move_from(small_header_index&& other) {
		headers = std::move(other.headers);
		alloc = std::move(other.alloc);
		_move_from(other);
	}

	void destroy() {
		std::for_each(edge_left, edge_right, deallocate_area<allocator>(alloc));
	}

	void _init() {
		std::tie(edge_left, edge_right) = middle_edges(headers);
		set_area(*edge_left, alloc.allocate(1));
		set_begin_end_indices(*edge_left, capacity(*edge_left));
	}

	void init() {
		headers.resize(8);
		_init();
	}

public:
	small_header_index(const allocator& alloc = allocator()) : alloc(alloc) { init(); }
	small_header_index(allocator&& alloc) : alloc(std::move(alloc)) { init(); }
	small_header_index(small_header_index&& other) :
		headers(std::move(other.headers)),
		alloc(std::move(other.alloc))
	{ 
		_move_from(other);
	}
	small_header_index(const small_header_index& other) :
		headers(other.size() + (other.size() >> 1)),
	    alloc(other.alloc)
	{ 
		_init();
	}
	~small_header_index() { destroy(); }

	small_header_index& operator=(small_header_index&& other) {
		destroy();
		move_from(std::move(other));
		return *this;
	};
	small_header_index& operator=(const small_header_index& other) {
		destroy();
		headers.resize(other.size() >> 1);
		alloc = other.alloc();
		_init();
		return *this;
	};

	iterator insert(iterator it, size_type n) {
		// precondition: it belongs to [begin(), end()]
		insert_headers_and_allocate_areas(headers, edge_left, edge_right, alloc, it, n);
		return it;
	}

	iterator erase(iterator first, iterator last) {
		// precondition: [first, last] belongs to [begin(), end()]
		erase_headers_and_deallocate_areas(edge_left, edge_right, alloc, first, last);
		return first;
	}

	void clear() {
		erase(begin(), end());
	}

	friend
	iterator insert(small_header_index& i, iterator it, size_type n) {
		return i.insert(it, n);
	}

	friend
	iterator erase(small_header_index& i, iterator first, iterator last) {
		return i.erase(first, last);
	}

	friend
	iterator insert(small_header_index& i, iterator it) {
		return i.insert(it, 1);
	}

	friend
	iterator erase(small_header_index& i, iterator it) {
		return i.erase(it, it + 1);
	}

	iterator begin() { return edge_left; }
	const_iterator cbegin() const { return const_iterator(edge_left); }
	const_iterator begin() const { return cbegin(); }

	iterator end() { return edge_right - 1; }
	const_iterator cend() const { return const_iterator(edge_right - 1); }
	const_iterator end() const { return cend(); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
	const_reverse_iterator rbegin() const { return crbegin(); }

	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
	const_reverse_iterator rend() const { return crend(); }

	size_t size() const { return static_cast<size_t>(end() - begin()); }
	bool empty() const { return cbegin() == cend(); }
};









template<typename T, typename I>
// I models SegmentIndex
class list_tmp
{
public:
	using index = I;
	using allocator = AllocatorType<index>;
	using value_type = ValueType<index>;
	using header_iterator = Iterator<index>;
	using const_header_iterator = ConstIterator<index>;
	using flat_iterator = value_type*;
	using const_flat_iterator = value_type *;
	using segment_iterator = seg::segment_iterator<header_iterator>;
	using const_segment_iterator = seg::const_segment_iterator<header_iterator, const_header_iterator>;
	using segmented_coordinate = seg::segmented_coordinate<segment_iterator, const_segment_iterator>;
	using const_segmented_coordinate = seg::const_segmented_coordinate<segment_iterator, const_segment_iterator>;
	using iterator = segmented_coordinate;
	using size_type = seg::size_t;
	using find_adaptor = flat::find_adaptor_binary;
	using equal_range_find_adaptor = flat::equal_range_adaptor_binary;

	static segmented_coordinate coordinate_from_const(const_segmented_coordinate it) {
		return segmented_coordinate(segment_iterator_from_const(it._seg), const_cast<flat_iterator>(it._flat));
	}

private:
	index in;
	size_type s;

	segment_iterator segment_iterator_from_const(const_segment_iterator it) {
		return segment_iterator(flat::successor(std::begin(in), it.h - std::cbegin(in)));
	}
	std::pair<header_iterator, size_t> header_from_coordinate(segmented_coordinate c) {
		if (c._seg.h != in.end() || empty())
			return { header_iterator(c._seg.h), static_cast<size_t>(c._flat - seg::begin(*c._seg.h)) };
		return { header_iterator(c._seg.h - 1), seg::size(*(c._seg.h - 1)) };
	}


	const_segmented_coordinate coordinate_unguarded(const std::pair<header_iterator, size_t>& r) const {
		if (r.second == seg::size(*r.first))
			return const_segmented_coordinate(const_segment_iterator(r.first + 1), seg::begin(*(r.first + 1)));
		return const_segmented_coordinate(const_segment_iterator(r.first), flat::successor(seg::cbegin(*r.first), r.second));
	}
	segmented_coordinate coordinate_unguarded(const std::pair<header_iterator, size_t>& r) {
		if (r.second == seg::size(*r.first))
			return segmented_coordinate(segment_iterator(r.first + 1), seg::begin(*(r.first + 1)));
		return segmented_coordinate(segment_iterator(r.first), flat::successor(seg::begin(*r.first), r.second));
	}
	const_segmented_coordinate coordinate(const std::pair<header_iterator, size_t>& r) const {
		if (r.first == std::end(in))
			return const_segmented_coordinate(const_segment_iterator(r.first), seg::begin(*r.first));
		return coordinate_unguarded(r);
	}
	segmented_coordinate coordinate(const std::pair<header_iterator, size_t>& r) {
		if (r.first == std::end(in))
			return segmented_coordinate(segment_iterator(r.first), seg::begin(*r.first));
		return coordinate_unguarded(r);
	}


	segmented_coordinate __insert(std::pair<header_iterator, seg::size_t> it) {
		segmented_coordinate _it = coordinate_unguarded(seg::insert_to_segment_range(in, it.first, it.second, 1).first);
		s = s + 1;
		return _it;
	}
	std::pair<segmented_coordinate, segmented_coordinate> __insert(std::pair<header_iterator, seg::size_t> it, size_type n) {
		auto [_begin, _end] = seg::insert_to_segment_range(in, it.first, it.second, static_cast<seg::size_t>(n));
		s = s + n;
		return { coordinate_unguarded(_begin), coordinate_unguarded(_end) };
	}


	segmented_coordinate _insert(segmented_coordinate it) {
		return __insert(header_from_coordinate(it));
	}
	std::pair<segmented_coordinate, segmented_coordinate> _insert(segmented_coordinate it, size_type n) {
		return __insert(header_from_coordinate(it), n);
	}


	segmented_coordinate __erase(std::pair<header_iterator, seg::size_t> left, std::pair<header_iterator, seg::size_t> right) {
		auto [it, i, _s] = seg::erase_from_segment_range(in, left.first, left.second, right.first, right.second);
		s = s - _s;
		return coordinate(std::make_pair(it, i));
	}


	segmented_coordinate _erase(segmented_coordinate left, segmented_coordinate right) {
		return __erase(header_from_coordinate(left), header_from_coordinate(right));
	}


	segmented_coordinate _erase(segmented_coordinate left) {
		return __erase(header_from_coordinate(left), header_from_coordinate(successor(left, 1)));
	}

	void copy_from(const list_tmp& other) {
		s = other.s;
		seg::copy(std::begin(other), std::end(other), _insert(std::begin(in), s));
	}

public:
	list_tmp(allocator&& alloc = allocator()) : in(std::move(alloc)), s(0) {}
	list_tmp(list_tmp&& other) : in(std::move(other.in)), s(std::move(other.s)), cmp(std::move(other.cmp)) {}
	list_tmp(const allocator& alloc) : in(alloc), s(0) {}
	list_tmp(const list_tmp& other) : in(other.in), s(other.s), cmp(other.cmp) { copy_from(other); }
	~list_tmp() { clear(); }

	list_tmp& operator=(list_tmp&& other) {
		clear();
		in = std::move(other.in);
		s = std::move(other.s);
		return *this;
	}
	list_tmp& operator=(const list_tmp& other) {
		clear();
		copy_from();
		return *this;
	}

	friend
	bool operator==(const list_tmp& x, const list_tmp& y) {
		if (x.size() != y.size()) return false;
		return seg::equal(std::cbegin(x), std::cend(x), std::cbegin(y));
	}
	friend
	bool operator!=(const list_tmp& x, const list_tmp& y) {
		return !(x == y);
	}

	friend
	bool operator<(const list_tmp& x, const list_tmp& y) {
		return seg::lexicographical_compare(std::cbegin(x), std::cend(x), std::cbegin(y), std::cend(y)) < 0;
	}
	friend
	bool operator>(const list_tmp& x, const list_tmp& y) {
		return y < x;
	}
	friend
	bool operator<=(const list_tmp& x, const list_tmp& y) {
		return !(y < x);
	}
	friend
	bool operator>=(const list_tmp& x, const list_tmp& y) {
		return !(x < y);
	}

	bool empty() const { return in.empty(); }

	size_type size() const { return s; }

	void swap(index& _in, size_type& _s) {
		std::swap(_in, in);
		std::swap(_s, s);
	}

	segmented_coordinate begin() {
		segment_iterator fseg = segment_iterator(std::begin(in));
		return segmented_coordinate(fseg, std::begin(fseg));
	}
	segmented_coordinate end() {
		segment_iterator lseg = segment_iterator(std::end(in));
		return segmented_coordinate(lseg, std::begin(lseg));
	}
	const_segmented_coordinate cbegin() {
		const_segment_iterator fseg = const_segment_iterator(std::begin(in));
		return const_segmented_coordinate(fseg, const_flat_iterator(std::begin(fseg)));
	}
	const_segmented_coordinate cend() {
		const_segment_iterator lseg = const_segment_iterator(std::end(in));
		return const_segmented_coordinate(lseg, const_flat_iterator(std::begin(lseg)));
	}
	const_segmented_coordinate begin() const { return cbegin(); }
	const_segmented_coordinate end() const { return cend(); }

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_move(segmented_coordinate it, I first, size_type n) {
		std::pair<segmented_coordinate, segmented_coordinate> r = _insert(it, static_cast<seg::size_t>(n));
		seg::move_flat_n_seg_uninitialized(first, n, r.first);
		return r;
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert(segmented_coordinate it, I first, size_type n) {
		std::pair<segmented_coordinate, segmented_coordinate> r = _insert(it, static_cast<seg::size_t>(n));
		seg::copy_flat_n_seg_uninitialized(first, n, r.first);
		return r;
	}

	segmented_coordinate insert(segmented_coordinate it, value_type&& v) {
		it = _insert(it);
		construct_at(it, std::move(v));
		return it;
	}

	segmented_coordinate insert(segmented_coordinate it, const value_type& v) {
		it = _insert(it);
		construct_at(it, v);
		return it;
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_move(const_segmented_coordinate it, I first, size_type n) {
		return insert_move(coordinate_from_const(it), first, n);
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert(const_segmented_coordinate it, I first, size_type n) {
		return insert(coordinate_from_const(it), first, n);
	}

	segmented_coordinate insert(const_segmented_coordinate it, value_type&& v) {
		return insert(coordinate_from_const(it), std::move(v));
	}
	segmented_coordinate insert(const_segmented_coordinate it, const value_type& v) {
		return insert(coordinate_from_const(it), v);
	}

	segmented_coordinate erase(segmented_coordinate first, segmented_coordinate last) {
		return _erase(first, last);
	}
	segmented_coordinate erase(segmented_coordinate it) {
		return _erase(it);
	}

	void clear() {
		_erase(begin(), end());
		s = 0;
	}

	segmented_coordinate erase(const_segmented_coordinate first, const_segmented_coordinate last) {
		return erase(coordinate_from_const(first), coordinate_from_const(last));
	}
	segmented_coordinate erase(const_segmented_coordinate it) {
		return erase(coordinate_from_const(it));
	}
};


constexpr size_t default_segment_byte_capacity = 65536u;

template<typename T>
constexpr segment_size_t default_capacity_for_type() {
	constexpr segment_size_t size = sizeof(T);
	constexpr segment_size_t capacity = static_cast<segment_size_t>(default_segment_byte_capacity / size);
	return capacity > 2 ? capacity : 2;
}

template<typename T, std::size_t C, typename A>
using list_big_header = list_tmp<T, big_header_index<T, C, A>>;

template<typename T, std::size_t C, typename A>
using list_small_header = list_tmp<T, small_header_index<T, C, A>>;

template<typename T, std::size_t C, typename A>
using list = list_big_header<T, C, A>;




template<typename K, typename M, typename Cmp, typename SList, typename CmpAdapt, typename VK, typename FAdaptor, typename EqualRangeFAdaptor>
// SList models SegmentedList
// Cmp models StrictWeakOrdering
// Domain<Cmp> == K
// S models BoundedSearch
class associative_container_tmp
{
public:
	using value_to_key = VK;
	using compare_adaptor = CmpAdapt;
	using key_type = K;
	using mapped_type = M;
	using segmented_list = SList;
	using key_compare = Cmp;
	using index = Index<segmented_list>;
	using allocator = AllocatorType<segmented_list>;
	using value_type = ValueType<segmented_list>;
	using flat_iterator = FlatIterator<segmented_list>;
	using const_flat_iterator = ConstFlatIterator<segmented_list>;
	using segment_iterator = SegmentIterator<segmented_list>;
	using const_segment_iterator = ConstSegmentIterator<segmented_list>;
	using segmented_coordinate = SegmentedCoordinate<segmented_list>;
	using const_segmented_coordinate = ConstSegmentedCoordinate<segmented_list>;
	using iterator = segmented_coordinate;
	using size_type = seg::size_t;
	using find_adaptor = FAdaptor;
	using equal_range_find_adaptor = EqualRangeFAdaptor;

private:
	struct _move {
		template<typename T>
		T&& operator()(T& x) const {
			return std::move(x);
		}
	};

	struct _copy {
		template<typename T>
		const T& operator()(const T& x) const {
			return x;
		}
	};

	struct equal_adaptor
	{
		compare_adaptor cmp;
		equal_adaptor(compare_adaptor cmp) : cmp(cmp) {}

		bool operator()(const value_type& x, const value_type& y) const {
			return !cmp(x, y) && !cmp(y, x);
		}
	};

	segmented_list list;
	compare_adaptor cmp;

	template<typename I, typename C>
	void insert_sorted(segmented_coordinate it, I first, size_type n, C c) {
		if (n == 0) return;

		segmented_coordinate it = begin();
		while (n) {
			it = upper_bound(it, *first);
			it = insert_unguarded(it, c(*first));
			++it;
			++first;
		}
	}

public:
	associative_container_tmp(associative_container_tmp&& other) = default;
	associative_container_tmp(const associative_container_tmp& other) = default;
	associative_container_tmp(key_compare&& cmp = key_compare(), allocator&& alloc = allocator()) : list(std::move(alloc)), cmp(std::move(cmp)) {}
	associative_container_tmp(const key_compare& cmp, const allocator& alloc) : list(alloc), cmp(cmp) {}
	associative_container_tmp(key_compare&& cmp, segmented_list&& list) : list(std::move(list)), cmp(std::move(cmp)) {}
	associative_container_tmp(const key_compare& cmp, const segmented_list& list) : list(list), cmp(cmp) {}
	~associative_container_tmp() = default;

	associative_container_tmp& operator=(associative_container_tmp&& other) = default;
	associative_container_tmp& operator=(const associative_container_tmp& other) = default;

	friend
	bool operator==(const associative_container_tmp& x, const associative_container_tmp& y) {
		if (x.size() != y.size()) return false;
		return seg::equal(std::cbegin(x), std::cend(x), std::cbegin(y), equal_adaptor(cmp));
	}
	friend
	bool operator!=(const associative_container_tmp& x, const associative_container_tmp& y) {
		return !(x == y);
	}

	friend
	bool operator<(const associative_container_tmp& x, const associative_container_tmp& y) {
		return seg::lexicographical_compare(std::cbegin(x), std::cend(x), std::cbegin(y), std::cend(y), cmp) < 0;
	}
	friend
	bool operator>(const associative_container_tmp& x, const associative_container_tmp& y) {
		return y < x;
	}
	friend
	bool operator<=(const associative_container_tmp& x, const associative_container_tmp& y) {
		return !(y < x);
	}
	friend
	bool operator>=(const associative_container_tmp& x, const associative_container_tmp& y) {
		return !(x < y);
	}

	bool empty() const { return list.empty(); }

	size_type size() const { return list.size(); }

	key_compare key_comp() const { return cmp.key_comp(); }

	void swap(segmented_list& _list) {
	    std::swap(_list, list);
	}

	void swap(index& _in, size_type& _s) {
		list.swap(_in, _s);
	}

	segmented_coordinate begin() { return list.begin(); }
	segmented_coordinate end() { return list.end(); }

	const_segmented_coordinate cbegin() { return list.cbegin(); }
	const_segmented_coordinate cend() { return list.cend(); }

	const_segmented_coordinate begin() const { return cbegin(); }
	const_segmented_coordinate end() const { return cend(); }

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_move_sorted_unguarded(segmented_coordinate it, I first, size_type n) {
		return list.insert_move(it, first, n);
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_sorted_unguarded(segmented_coordinate it, I first, size_type n) {
		return list.insert(it, first, n);
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	void insert_sorted(I first, size_type n) {
		insert_sorted(first, n, _copy());
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	void insert_move_sorted(I first, size_type n) {
		insert_sorted(first, n, _move());
	}

	segmented_coordinate insert_unguarded(segmented_coordinate it, value_type&& v) {
		return list.insert(it, std::move(v));
	}

	segmented_coordinate insert_unguarded(segmented_coordinate it, const value_type& v) {
		return list.insert(it, v);
	}

	segmented_coordinate insert(value_type&& v) {
		return insert_unguarded(upper_bound(value_to_key::get(v)), std::move(v));
	}

	segmented_coordinate insert(const value_type& v) {
		return insert_unguarded(upper_bound(value_to_key::get(v)), v);
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_move_sorted_unguarded(const_segmented_coordinate it, I first, size_type n) {
		return list.insert_move(it, first, n);
	}

	template<typename I>
	// I models InnputIterator
	// IteratorValueType<I> == value_type
	std::pair<segmented_coordinate, segmented_coordinate> insert_sorted_unguarded(const_segmented_coordinate it, I first, size_type n) {
		return list.insert(it, first, n);
	}

	segmented_coordinate insert_unguarded(const_segmented_coordinate it, value_type&& v) {
		return list.insert(it, std::move(v));
	}
	segmented_coordinate insert_unguarded(const_segmented_coordinate it, const value_type& v) {
		return list.insert(it, v);
	}

	segmented_coordinate erase(segmented_coordinate first, segmented_coordinate last) {
		return list.erase(first, last);
	}
	segmented_coordinate erase(segmented_coordinate it) {
		return list.erase(it);
	}

	void clear() {
		list.clear();
	}

	segmented_coordinate erase(const_segmented_coordinate first, const_segmented_coordinate last) {
		return list.erase(first, last);
	}
	segmented_coordinate erase(const_segmented_coordinate it) {
		return list.erase(it);
	}


	segmented_coordinate lower_bound(const key_type& k) {
		return seg::lower_bound(begin(), end(), k, cmp, find_adaptor());
	}
	const_segmented_coordinate lower_bound(const key_type& k) const {
		return seg::lower_bound(cbegin(), cend(), k, cmp, find_adaptor());
	}

	segmented_coordinate upper_bound(const key_type& k) {
		return seg::upper_bound(begin(), end(), k, cmp, find_adaptor());
	}
	const_segmented_coordinate upper_bound(const key_type& k) const {
		return seg::upper_bound(cbegin(), cend(), k, cmp, find_adaptor());
	}

	std::pair<segmented_coordinate, segmented_coordinate> equal_range(const key_type& k) {
		return seg::equal_range(begin(), end(), k, cmp, equal_range_find_adaptor());
	}
	std::pair<const_segmented_coordinate, const_segmented_coordinate> equal_range(const key_type& k) const {
		return seg::equal_range(cbegin(), cend(), k, cmp, equal_range_find_adaptor());
	}

	segmented_coordinate lower_bound(segmented_coordinate it, const key_type& k) {
		return seg::lower_bound(it, end(), k, cmp, find_adaptor());
	}
	segmented_coordinate lower_bound(const_segmented_coordinate it, const key_type& k) {
		return seg::lower_bound(listtor::coordinate_from_const(it), end(), k, cmp, find_adaptor());
	}
	const_segmented_coordinate lower_bound(const_segmented_coordinate it, const key_type& k) const {
		return seg::lower_bound(it, cend(), k, cmp, find_adaptor());
	}

	segmented_coordinate upper_bound(segmented_coordinate it, const key_type& k) {
		return seg::upper_bound(it, end(), k, cmp, find_adaptor());
	}
	segmented_coordinate upper_bound(const_segmented_coordinate it, const key_type& k) {
		return seg::upper_bound(listtor::coordinate_from_const(it), end(), k, cmp, find_adaptor());
	}
	const_segmented_coordinate upper_bound(const_segmented_coordinate it, const key_type& k) const {
		return seg::upper_bound(it, cend(), k, cmp, find_adaptor());
	}

	std::pair<segmented_coordinate, segmented_coordinate> equal_range(segmented_coordinate it, const key_type& k) {
		return seg::equal_range(it, end(), k, cmp, find_adaptor());
	}
	std::pair<segmented_coordinate, segmented_coordinate> equal_range(const_segmented_coordinate it, const key_type& k) {
		return seg::equal_range(listtor::coordinate_from_const(it), end(), k, cmp, find_adaptor());
	}
	std::pair<const_segmented_coordinate, const_segmented_coordinate> equal_range(const_segmented_coordinate it, const key_type& k) const {
		return seg::equal_range(it, cend(), k, cmp, find_adaptor());
	}
};

template<typename Cmp>
// Cmp models StrictWeakOrdering
struct map_compare_adaptor
{
	Cmp cmp;

	map_compare_adaptor(const Cmp& cmp) : cmp(cmp) {}

	template<typename T>
	// Domain<Cmp> == typename T::first
	bool operator()(const T& x, const T& y) const {
		return cmp(x.first, y.first);
	}

	Cmp key_compare() const { return cmp; }
};

template<typename Cmp>
// Cmp models StrictWeakOrdering
struct set_compare_adaptor
{
	Cmp cmp;

	set_compare_adaptor(const Cmp& cmp) : cmp(cmp) {}

	template<typename T>
	// Domain<Cmp> == T
	bool operator()(const T& x, const T& y) const {
		return cmp(x, y);
	}

	Cmp key_compare() const { return cmp; }
};

struct map_value_to_key
{
	template<typename T0, typename T1>
	static T0& get(std::pair<T0, T1>& x) { return x.first; }

	template<typename T0, typename T1>
	static const T0& get(const std::pair<T0, T1>& x) { return x.first; }
};

struct set_value_to_key
{
	template<typename T>
	static T& get(T& x) { return x; }

	template<typename T>
	static const T& get(const T& x) { return x; }
};

template<
	typename K,
	typename M,
	typename Cmp,
	typename SList,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multimap_tmp = associative_container_tmp<K, M, Cmp, SList, map_compare_adaptor<Cmp>, map_value_to_key, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename M,
	typename Cmp,
	segment_size_t C,
	typename A,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multimap_big_header = multimap_tmp<K, M, Cmp, list_big_header<std::pair<K, M>, C, A>, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename M,
	typename Cmp,
	segment_size_t C,
	typename A,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multimap_small_header = multimap_tmp<K, M, Cmp, list_small_header<std::pair<K, M>, C, A>, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename M,
	typename Cmp = std::less<K>,
	segment_size_t C = default_capacity_for_type<std::pair<K, M>>(),
	typename A = std::allocator<std::pair<K, M>>>
using multimap = multimap_big_header<K, M, Cmp, C, A, flat::find_adaptor_linear, flat::equal_range_adaptor_linear>;

template<
	typename K,
	typename Cmp,
	typename SList,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multiset_tmp = associative_container_tmp<K, K, Cmp, SList, set_compare_adaptor<Cmp>, set_value_to_key, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename Cmp,
	segment_size_t C,
	typename A,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multiset_big_header = multiset_tmp<K, Cmp, list_big_header<K, C, A>, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename Cmp,
	segment_size_t C,
	typename A,
	typename FAdaptor,
	typename EqualRangeFAdaptor>
using multiset_small_header = multiset_tmp<K, Cmp, list_small_header<K, C, A>, FAdaptor, EqualRangeFAdaptor>;

template<
	typename K,
	typename Cmp = std::less<K>,
	segment_size_t C = default_capacity_for_type<K>(),
	typename A = std::allocator<K>>
using multiset = multiset_big_header<K, Cmp, C, A, flat::find_adaptor_linear, flat::equal_range_adaptor_linear>;

} // namespace seg



} // namespace str2d