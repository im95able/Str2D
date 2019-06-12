#pragma once

#include <utility>
#include <algorithm>
#include <cstring>
#include <functional>
#include "utility.h"

namespace flat
{



struct find_adaptor_linear
{
	template<typename I, typename Pred>
	// I models ForwardIterator
	// Pred models UnaryPredicate
	// IteratorValueType<I> == Domain<Pred>
	I operator()(I first, I last, Pred p) const {
		return std::find_if(first, last, p);
	}
};

struct find_adaptor_binary
{
	template<typename I, typename Pred>
	// I models ForwardIterator
	// Pred models UnaryPredicate
	// IteratorValueType<I> == Domain<Pred>
	I operator()(I first, I last, Pred p) const {
		return std::partition_point(first, last, p);
	}
};

struct equal_range_adaptor_linear
{
	template<typename I, typename Cmp>
	// I models ForwardIterator
	// Cmp models StrictWeakOrdering
	// IteratorValueType<I> == Domain<Cmp>
	std::pair<I, I> operator()(I first, I last, const IteratorValueType<I>& x, Cmp cmp) const {
		I lower_bound = std::find_if(first, last, lower_bound_predicate<IteratorValueType<I>, Cmp>(x, cmp));
		return { lower_bound, std::find_if(lower_bound, last, upper_bound_predicate<IteratorValueType<I>, Cmp>(x, cmp)) };
	}
};

struct equal_range_adaptor_binary
{
	template<typename I, typename Cmp>
	// I models ForwardIterator
	// Cmp models StrictWeakOrdering
	// IteratorValueType<I> == Domain<Cmp>
	std::pair<I, I> operator()(I first, I last, const IteratorValueType<I>& x, Cmp cmp) const {
		return std::equal_range(first, last, x, cmp);
	}
};


template<typename T, typename N>
// T models TriviallyCopyable
// N models Integer
inline
std::pair<T*, T*> memmove_n(T* first0, N n, T* first1) {
	std::memmove(static_cast<void*>(first1), static_cast<void*>(first0), n * sizeof(T));
	return { first0 + IteratorDifferenceType<T*>(n), first1 + IteratorDifferenceType<T*>(n) };
}

template<typename T>
// T models TriviallyCopyable
inline
T* memmove(T* first0, T* last0, T* first1) { return flat::memmove_n(first0, last0 - first0, first1).second; }

template<typename T, typename N>
// T models TriviallyCopyable
// N models Integer
inline
std::pair<T*, T*> memmove_backward_n(T* last0, N n, T* last1) {
	std::memmove(static_cast<void*>(last1 - IteratorDifferenceType<T*>(n)), static_cast<void*>(last0 - IteratorDifferenceType<T*>(n)), n * sizeof(T));
	return { last0 - IteratorDifferenceType<T*>(n), last1 - IteratorDifferenceType<T*>(n) };
}

template<typename T>
// T models TriviallyCopyable
inline
T* memmove_backward(T* first0, T* last0, T* last1) { return flat::memmove_backward_n(last0, last0 - first0, last1).second; }


template<typename I>
// I models InputIterator
void destruct(I first, I last) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while(first != last) destruct_at(first++);
	}
}

template<typename I, typename N>
// I models InputIterator
// N models Integer
void destruct_n(I first, N n) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while(n) {
			destruct_at(first++);
			--n;
		}
	}
}

template<typename I>
// I models BidirectionalIterator
void destruct_backward(I first, I last) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while(first != last) destruct_at(--last);
	}
}

template<typename I, typename N>
// I models BidirectionalIterator
// N models Integer
void destruct_backward_n(I last, N n) {
	if constexpr(!TriviallyDestructible<IteratorValueType<I>>::value) {
		while(n) {
			destruct_at(--last);
			--n;
		}
	}
}


template<typename I, typename N>
// I models Iterator
// N models Integer
inline
I successor(I it, N n) {
	return std::next(it, static_cast<IteratorDifferenceType<I>>(n));
}

template<typename I, typename N>
// I models Iterator
// N models Integer
inline
I predecessor(I it, N n) {
	return std::prev(it, static_cast<IteratorDifferenceType<I>>(n));
}


template<typename I0, typename I1, typename C>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
I1 _copy(I0 first0, I0 last0, I1 first1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		return std::copy(first0, last0, first1);
	}
	else {
		while (first0 != last0) c(*first1++, *first0++);
		return first1;
	}
}

template<typename I0, typename N, typename I1, typename C>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
std::pair<I0, I1> _copy_n(I0 first0, N n, I1 first1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		std::copy_n(first0, n, first1);
		return { flat::successor(first0, n), flat::successor(first1, n) };
	}
	else {
		while (n > 0) {
			c(*first1++, *first0++);
			--n;
		}
		return { first0, first1 };
	}
}

template<typename I0, typename I1, typename C>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
I1 _copy_backward(I0 first0, I0 last0, I1 last1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		return std::copy_backward(first0, last0, last1);
	}
	else {
		while (first0 != last0) c(*--last1, *--last0);
		return last1;
	}
}

template<typename I0, typename N, typename I1, typename C>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
std::pair<I0, I1> _copy_backward_n(I0 last0, N n, I1 last1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		I0 first0 = flat::predecessor(last0, n);
		return { first0, flat::_copy_backward(first0, last0, last1, c) };
	}
	else {
		while (n) {
			c(*--last1, *--last0);
			--n;
		}
		return { last0, last1 };
	}
}

template<typename I0, typename I1, typename C>
// I0 models ForwardIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
inline
I1 _cut(I0 first0, I1 last0, I1 first1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		return std::copy(first0, last0, first1);
	}
	else {
		while (first0 != last0) {
			c(*first1++, *first0);
			destruct_at(first0++);
		}
		return first1;
	}
}

template<typename I0, typename N, typename I1, typename C>
// I0 models ForwardIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
inline
std::pair<I0, I1> _cut_n(I0 first0, N n, I1 first1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		return std::copy_n(first0, n, first1);
	}
	else {
		while (n > 0) {
			c(*first1, *first0);
			destruct_at(first0);
			++first0;
			++first1;
			--n;
		}
		return { first0, first1 };
	}
}

template<typename I0, typename I1, typename C>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
I1 _cut_backward(I0 first0, I0 last0, I1 last1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		return std::copy_backward(first0, last0, last1);
	}
	else {
		while (first0 != last0) {
			c(*--last1, *--last0);
			destruct_at(last0);
		}
		return last1;
	}
}

template<typename I0, typename N, typename I1, typename C>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
// C models Procedure
// Arrity<C> == 2
// InputType<C, 0> == IteratorValueType<I1>
// InputType<C, 1> == IteratorValueType<I0>
std::pair<I0, I1> _cut_backward_n(I0 last0, N n, I1 last1, C c) {
	if constexpr (TriviallyCopyableMemory<I0, I1>::value) {
		I0 first0 = flat::predecessor(last0, n);
		return { first0, flat::_cut_backward(first0, last0, last1, c) };
	}
	else {
		while (n) {
			c(*--last1, *--last0);
			destruct_at(last0);
			--n;
		}
		return { last1, last0 };
	}
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_uninitialized(I0 first0, I1 last0, I1 first1) {
	return _copy(first0, last0, first1, move_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> move_n_uninitialized(I0 first0, N n, I1 first1) {
	return _copy_n(first0, n, first1, move_constructor());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_backward_uninitialized(I0 first0, I0 last0, I1 last1) {
	return _copy_backward(first0, last0, last1, move_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> move_backward_n_uninitialized(I0 last0, N n, I1 last1) {
	return _copy_backward_n(last0, n, last1, move_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move(I0 first0, I0 last0, I1 first1) {
	return _copy(first0, last0, first1, move_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> move_n(I0 first0, N n, I1 first1) {
	return _copy_n(first0, n, first1, move_assignment());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 move_backward(I0 first0, I0 last0, I1 last1) {
	return _copy_backward(first0, last0, last1, move_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> move_backward_n(I0 last0, N n, I1 last1) {
	return _copy_backward_n(last0, n, last1, move_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_uninitialized(I0 first0, I0 last0, I1 first1) {
	return _copy(first0, last0, first1, copy_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> copy_n_uninitialized(I0 first0, N n, I1 first1) {
	return _copy_n(first0, n, first1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_backward_uninitialized(I0 first0, I0 last0, I1 last1) {
	return _copy_backward(first0, last0, last1, copy_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> copy_backward_n_uninitialized(I0 last0, N n, I1 last1) {
	return _copy_backward_n(last0, n, last1, copy_constructor());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy(I0 first0, I0 last0, I1 first1) {
	return _copy(first0, last0, first1, copy_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> copy_n(I0 first0, N n, I1 first1) {
	return _copy_n(first0, n, first1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
I1 copy_backward(I0 first0, I0 last0, I1 last1) {
	return _copy_backward(first0, last0, last1, copy_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
inline
std::pair<I0, I1> copy_backward_n(I0 last0, N n, I1 last1) {
	return _copy_backward_n(last0, n, last1, copy_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
I1 cut(I0 first0, I0 last0, I1 first1) {
	return _cut(first0, last0, first1, move_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
std::pair<I0, I1> cut_n(I0 first0, N n, I1 first1) {
	return _cut_n(first0, n, first1, move_assignment());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
I1 cut_backward(I0 first0, I0 last0, I1 last1) {
	return _cut_backward(first0, last0, last1, move_assignment());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
std::pair<I0, I1> cut_backward_n(I0 last0, N n, I1 last1) {
	return _cut_backward_n(last0, n, last1, move_assignment());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
I1 cut_uninitialized(I0 first0, I0 last0, I1 first1) {
	return _cut(first0, last0, first1, move_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models OutputIterator
// IteratorValueType<I0> == IteratorValueType<I1>
std::pair<I0, I1> cut_n_uninitialized(I0 first0, N n, I1 first1) {
	return _cut_n(first0, n, first1, move_constructor());
}

template<typename I0, typename I1>
// I0 models BidirectionalIterator
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
I1 cut_backward_uninitialized(I0 first0, I0 last0, I1 last1) {
	return _cut_backward(first0, last0, last1, move_constructor());
}

template<typename I0, typename N, typename I1>
// I0 models BidirectionalIterator
// N models Integer
// I1 models BidirectionalIterator
// IteratorValueType<I0> == IteratorValueType<I1>
std::pair<I0, I1> cut_backward_n_uninitialized(I0 last0, N n, I1 last1) {
	return _cut_backward_n(last0, n, last1, move_constructor());
}

template<typename I, typename N0, typename N1>
// I models RandomAccessIterator
// N0 models Integer
// N1 models Integer
inline
void slide_cut_n(I first, N0 n, N1 move) {
	if constexpr(TriviallyCopyableMemory<I, I>::value) {
		std::copy_n(first, n, flat::predecessor(first, move));
	}
	else {
		if (n <= move) {
			flat::cut_n_uninitialized(first, n, flat::predecessor(first, move));
		}
		else {
			flat::move_n_uninitialized(first, move, flat::predecessor(first, move));
			flat::move_n(flat::successor(first, move), n - move, first);
			flat::destruct_n(flat::successor(first, n - move), move);
		}
	}
}

template<typename I, typename N0, typename N1>
// I models RandomAccessIterator
// N0 models Integer
// N1 models Integer
inline
void slide_cut_backward_n(I last, N0 n, N1 move) {
	if constexpr(TriviallyCopyableMemory<I, I>::value) {
		std::copy_backward(flat::predecessor(last, n), last, flat::successor(last, move));
	}
	else {
		if (n <= move) {
			flat::cut_backward_n_uninitialized(last, n, flat::successor(last, move));
		}
		else {
			flat::move_backward_n_uninitialized(last, move, flat::successor(last, move));
			flat::move_backward_n(flat::predecessor(last, move), n - move, last);
			flat::destruct_backward_n(flat::predecessor(last, n - move), move);
		}
	}
}


template<typename I0, typename N, typename I1, typename Cmp>
// I0 models InputIterator
// N models Integer
// I1 models InputIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == ValueType<I0> == ValueType<I1>
int compare_n(I0 first0, N n, I1 first1, Cmp cmp) {
	while (n) {
		if (cmp(*first0, *first1)) return -1;
		if (cmp(*first1, *first0)) return 1;
		++first0;
		++first1;
		--n;
	}
	return 0;
}

template<typename I0, typename I1, typename Cmp>
// I0 models InputIterator
// I1 models InputIterator
// Cmp models StrictWeakOrdering
// Domain<Cmp> == ValueType<I0> == ValueType<I1>
int compare(I0 first0, I0 last0, I1 first1, Cmp cmp) {
	while (first0 != last0) {
		if (cmp(*first0, *first1)) return -1;
		if (cmp(*first1, *first0)) return 1;
		++first0;
		++first1;
	}
	return 0;
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models InputIterator
// ValueType<I0> == ValueType<I1>
int compare_n(I0 first0, N n, I1 first1) {
	return compare_n(first0, n, first1, std::less<>());
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models InputIterator
// ValueType<I0> == ValueType<I1>
int compare(I0 first0, I0 last0, I1 first1) {
	return compare(first0, last0, first1, std::less<>());
}

template<typename I0, typename N, typename I1, typename P>
// I0 models InputIterator
// N models Integer
// I1 models InputIterator
// P models EquivalenceRelation
// Domain<P> == ValueType<I0> == ValueType<I1>
bool equal_n(I0 first0, N n, I1 first1, P p) {
	if constexpr (std::is_pointer_v<I0> && std::is_pointer_v<I1>) {
		return std::equal(first0, flat::successor(first0, n), first1, p);
	}
	else {
		while (n && p(*first0, *first1)) {
			++first0;
			++first1;
			--n;
		}
		return n == 0;
	}
}

template<typename I0, typename N, typename I1>
// I0 models InputIterator
// N models Integer
// I1 models InputIterator
// ValueType<I0> == ValueType<I1>
bool equal_n(I0 first0, N n, I1 first1) {
	return flat::equal_n(first0, n, first1, std::equal_to<>());
}

template<typename I0, typename I1, typename P>
// I0 models InputIterator
// I1 models InputIterator
// P models EquivalenceRelation
// Domain<P> == ValueType<I0> == ValueType<I1>
bool equal(I0 first0, I0 last0, I1 first1, P p) {
	if constexpr (std::is_pointer_v<I0> && std::is_pointer_v<I1>) {
		return std::equal(first0, last0, first1, p);
	}
	else {
		while (first0 != last0) {
			if (!p(*first0, *first1)) return { first0, last0 };
			++first0;
			++first1;
		}
		return first0 == last0;
	}
}

template<typename I0, typename I1>
// I0 models InputIterator
// I1 models InputIterator
// ValueType<I0> == ValueType<I1>
// ValueType<I0> models Regular
bool equal(I0 first0, I0 last0, I1 first1) {
	return flat::equal(first0, last0, first1, std::equal_to<>());
}



} // namespace flat
