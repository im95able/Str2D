#include <benchamark.h>
#include <btree_set.h>
#include <str2d.h>

#include "instrumented.h"

#include <chrono>
#include <random>
#include <set>
#include <vector>
#include <limits>
#include <algorithm>

#define POD_TEST      1
#define NON_POD_TEST  1

#define INSERT_SINGLE_TEST   0
#define ITERATE_TEST  0
#define LOOKUP_TEST  0
#define ERASE_SINGLE_TEST 0
#define INSERT_SORTED_UNGUARDED_TEST 0
#define ERASE_RANGE 1


#define _BENCHMARK_REGISTER_F(Fix, TestName, TimeUnit) BENCHMARK_REGISTER_F(Fix, TestName) \
	->Arg(1 << 16)  \
	->Arg(1	<< 27)	\
	->Unit(TimeUnit);

#define _BENCHMARK_REGISTER_SORTED_UNGUARDED_F(Fix, TestName, TimeUnit) BENCHMARK_REGISTER_F(Fix, TestName) \
	->Args({1 << 16, 1 << 2})   \
	->Args({1 << 16, 1 << 10})  \
	->Args({1 << 16, 1 << 18})  \
	->Args({1 << 27, 1 << 2})   \
	->Args({1 << 27, 1 << 10})  \
	->Args({1 << 27, 1 << 18})  \
	->Unit(TimeUnit)


using bint = std::int64_t;

template<typename T, std::size_t C>
using segmented_set_small_linear = str2d::seg::multiset_small_header<
	T, 
	std::less<T>, 
	C, 
	std::allocator<T>, 
	str2d::flat::find_adaptor_linear, 
	str2d::flat::equal_range_adaptor_linear>;

template<typename T, std::size_t C>
using segmented_set_small_binary = str2d::seg::multiset_small_header<
	T,
	std::less<T>,
	C,
	std::allocator<T>,
	str2d::flat::find_adaptor_binary,
	str2d::flat::equal_range_adaptor_binary>;

template<typename T, std::size_t C>
using segmented_set_big_linear = str2d::seg::multiset_big_header<
	T,
	std::less<T>,
	C,
	std::allocator<T>,
	str2d::flat::find_adaptor_linear,
	str2d::flat::equal_range_adaptor_linear>;

template<typename T, std::size_t C>
using segmented_set_big_binary = str2d::seg::multiset_big_header<
	T,
	std::less<T>,
	C,
	std::allocator<T>,
	str2d::flat::find_adaptor_binary,
	str2d::flat::equal_range_adaptor_binary>;


static std::default_random_engine rand_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
static std::uniform_int_distribution<bint> rand_int_distribution(std::numeric_limits<bint>::min(), std::numeric_limits<bint>::max());

template<typename T, std::size_t C>
using btree_set = btree::btree_multiset<T, std::less<T>, std::allocator<T>, C * sizeof(T)>;

inline
bint brand(bint min, bint max) {
	return std::uniform_int_distribution<bint>(min, max)(rand_engine);
}

inline
bint brand() {
	return rand_int_distribution(rand_engine);
}

struct difference_greater_than
{
	bint diff;
	difference_greater_than(bint diff) : diff(diff) {}

	bool operator()(bint x, bint y) const { return y - x > 1; }
};

class Fixture : public benchmark::Fixture {
public:
	Fixture() {}

	static constexpr std::size_t max_element_nm = 1 << 27;
	static constexpr std::size_t max_single_value_nm = 1 << 18;

	void SetUp(const ::benchmark::State& state) override {
#if POD_TEST
		unsorted.resize(max_element_nm);
		for (auto& v : unsorted) v = brand();
		sorted = unsorted;
		std::sort(sorted.begin(), sorted.end());
		single_insert_value.resize(max_single_value_nm);
		auto it = std::adjacent_find(str2d::flat::successor(sorted.begin(), sorted.size() >> 1), sorted.end(), difference_greater_than(1));
		bint v = it != sorted.end() ? (*it + 1) : (sorted.back() + 1);
		std::fill(single_insert_value.begin(), single_insert_value.end(), v);
#endif
	}

	void TearDown(const ::benchmark::State& state) override {

	}
	
#if POD_TEST
	static std::vector<bint> unsorted;
	static std::vector<bint> sorted;
	static std::vector<bint> single_insert_value;
#endif

#if NON_POD_TEST

#endif
};

std::vector<bint> Fixture::unsorted;
std::vector<bint> Fixture::sorted;
std::vector<bint> Fixture::single_insert_value;


template<typename C>
inline
void ConstructSetFromUnsorted(C& c, std::size_t s) {
	if constexpr (std::is_same_v<str2d::ValueType<C>, bint>) {
		for (std::size_t i = 0; i < s; ++i) 
			c.insert(Fixture::unsorted[i]);
	}
}

template<typename C>
inline
void ConstructSegmentedSetFromSorted(C& set, std::size_t s) {
	if (Fixture::sorted.empty()) return;

	if constexpr (std::is_same_v<str2d::ValueType<C>, bint>) {
		set.insert_sorted_unguarded(set.upper_bound(Fixture::sorted.front()), Fixture::sorted.begin(), s);
	}
}

template<typename C>
inline
void ConstructSetFromSorted(C& set, std::size_t s) {
	if constexpr (std::is_same_v<str2d::ValueType<C>, bint>) {
		for (std::size_t i = 0; i < s; ++i)
			set.insert(Fixture::sorted[i]);
	}
}


template<typename C, typename I>
inline
str2d::Iterator<C> SetInsertSorted(C& set, str2d::Iterator<C> it, I first, I last) {
	while (first != last) {
		it = set.insert(it, *first);
		++it;
		++first;
	}
	return it;
}



#if INSERT_SINGLE_TEST

template<typename C>
inline
void SetInsertSingleLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));

	std::size_t i = 0;
	for (auto _ : state) {
		set.insert(Fixture::unsorted[i]);
		state.PauseTiming();
		set.erase(--set.upper_bound(Fixture::unsorted[i]));
		++i;
		state.ResumeTiming();
	}
}

template<typename C>
inline
void SegmentedSetInsertSingleLoop(C& set, benchmark::State& state) {
	ConstructSegmentedSetFromSorted(set, state.range(0));

	std::size_t i = 0;
	for (auto _ : state) {
		set.insert(Fixture::unsorted[i]);
		state.PauseTiming();
		set.erase(--set.upper_bound(Fixture::unsorted[i]));
		++i;
		state.ResumeTiming();
	}
}

BENCHMARK_DEFINE_F(Fixture, SetInsertSingle_INT64)(benchmark::State& state) {
	SetInsertSingleLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetInsertSingle_INT64_C32)(benchmark::State& state) {
	SetInsertSingleLoop(btree_set<std::int64_t, 32>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSingleLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}

#define _BENCHMARK_REGISTER_F_INSERT_SINGLE(Fix, TestName) _BENCHMARK_REGISTER_F(Fix, TestName, benchmark::kNanosecond);

_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SetInsertSingle_INT64)

_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, BtreeSetInsertSingle_INT64_C32)


_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SINGLE(Fixture, SegmentedSetInsertSingle_BIG_BINARY_INT64_C8192)



#endif // INSERT_SINGLE_TEST


#if ITERATE_TEST

struct accumulate_ints
{
	bint x{ 0 };
	bint operator()(bint y) { return x = x + y; }
};


template<typename C>
static
void SetIterateLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));

	for (auto _ : state) std::for_each(set.begin(), set.end(), accumulate_ints());
}

template<typename C>
static
void SegmentedSetIterateLoop(C& set, benchmark::State& state) {
	ConstructSegmentedSetFromSorted(set, state.range(0));

	for (auto _ : state) str2d::seg::for_each(set.begin(), set.end(), accumulate_ints());
}


BENCHMARK_DEFINE_F(Fixture, SetIterate_INT64)(benchmark::State& state) {
	SetIterateLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetIterate_INT64_C32)(benchmark::State& state) {
	SetIterateLoop(btree_set<std::int64_t, 32>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetIterateLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}


#define _BENCHMARK_REGISTER_F_ITERATE(Fix, TestName) _BENCHMARK_REGISTER_F(Fix, TestName, benchmark::kMillisecond);

_BENCHMARK_REGISTER_F_ITERATE(Fixture, SetIterate_INT64)

_BENCHMARK_REGISTER_F_ITERATE(Fixture, BtreeSetIterate_INT64_C32)

_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ITERATE(Fixture, SegmentedSetIterate_BIG_BINARY_INT64_C8192)

#endif // ITERATE_TEST


#if LOOKUP_TEST

template<typename C>
static
void SetLookupLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));
	std::size_t i = 0;
	for (auto _ : state) {
		benchmark::DoNotOptimize(set.lower_bound(Fixture::unsorted[i]));
		++i;
	}
}

template<typename C>
static
void SegmentedSetLookupLoop(C& set, benchmark::State& state) {
	ConstructSegmentedSetFromSorted(set, state.range(0));
	std::size_t i = 0;
	for (auto _ : state) {
		benchmark::DoNotOptimize(set.lower_bound(Fixture::unsorted[i]));
		++i;
	}
}

BENCHMARK_DEFINE_F(Fixture, SetLookup_INT64)(benchmark::State& state) {
	SetLookupLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetLookup_INT64_C32)(benchmark::State& state) {
	SetLookupLoop(btree_set<std::int64_t, 32>(), state);
}



BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_linear<std::int64_t, 8192>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetLookupLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}

#define _BENCHMARK_REGISTER_F_LOOKUP(Fix, TestName) _BENCHMARK_REGISTER_F(Fix, TestName, benchmark::kNanosecond);

_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SetLookup_INT64)

_BENCHMARK_REGISTER_F_LOOKUP(Fixture, BtreeSetLookup_INT64_C32)


_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_LOOKUP(Fixture, SegmentedSetLookup_BIG_BINARY_INT64_C8192)

#endif // LOOKUP_TEST



#if ERASE_SINGLE_TEST

template<typename C>
inline
void _SetEraseSingleLoop(C& set, benchmark::State& state) {
	std::size_t i = 0;
	str2d::Iterator<C> it;
	for (auto _ : state) {

		state.PauseTiming();
		it = --set.upper_bound(Fixture::unsorted[i]);
		state.ResumeTiming();

		set.erase(it);

		state.PauseTiming();
		set.insert(Fixture::unsorted[i]);
		++i;
		state.ResumeTiming();
	}
}

template<typename C>
inline
void SetEraseSingleLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));
	_SetEraseSingleLoop(set, state);
}

template<typename C>
inline
void SegmentedSetEraseSingleLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));
	_SetEraseSingleLoop(set, state);
}


BENCHMARK_DEFINE_F(Fixture, SetEraseSingle_INT64)(benchmark::State& state) {
	SetEraseSingleLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetEraseSingle_INT64_C32)(benchmark::State& state) {
	SetEraseSingleLoop(btree_set<std::int64_t, 32>(), state);
}


BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseSingleLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}

#define _BENCHMARK_REGISTER_F_ERASE(Fix, TestName) _BENCHMARK_REGISTER_F(Fix, TestName, benchmark::kNanosecond);

_BENCHMARK_REGISTER_F_ERASE(Fixture, SetEraseSingle_INT64, erase_unit)

_BENCHMARK_REGISTER_F_ERASE(Fixture, BtreeSetEraseSingle_INT64_C32)

_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE(Fixture, SegmentedSetEraseSingle_BIG_BINARY_INT64_C8192)

#endif // ERASE_SINGLE_TEST


#if INSERT_SORTED_UNGUARDED_TEST

template<typename C, typename I>
void SetInsertSortedUnguarded(C& set, I first, I last, benchmark::State& state) {
	str2d::IteratorDifferenceType<I> diff = last - first;
	str2d::Iterator<C> it = SetInsertSorted(set, set.begin(), first, last);
	state.PauseTiming();
	set.erase(str2d::flat::predecessor(it, diff), it);
	state.ResumeTiming();
}

template<typename C, typename I>
void SegmentedSetInsertSortedUnguarded(C& set, I first, I last, benchmark::State& state) {
	if (first == last) return;
	std::pair<str2d::Iterator<C>, str2d::Iterator<C>> p = set.insert_sorted_unguarded(set.upper_bound(*first), first, str2d::SizeType<C>(last - first));
	state.PauseTiming();
	set.erase(p.first, p.second);
	state.ResumeTiming();
}

template<typename C>
inline
void SetInsertSortedUnguardedLoop(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));
	for (auto _ : state) {
		SetInsertSortedUnguarded(
			set,
			Fixture::single_insert_value.begin(),
			str2d::flat::successor(Fixture::single_insert_value.begin(), state.range(1)),
			state);
	}
}

template<typename C>
inline
void SegmentedSetInsertSortedUnguardedLoop(C& set, benchmark::State& state) {
	ConstructSegmentedSetFromSorted(set, state.range(0));
	for (auto _ : state) {
		SegmentedSetInsertSortedUnguarded(
			set,
			Fixture::single_insert_value.begin(),
			str2d::flat::successor(Fixture::single_insert_value.begin(), state.range(1)),
			state);
	}
}


BENCHMARK_DEFINE_F(Fixture, SetInsertSortedUnguarded_INT64)(benchmark::State& state) {
	SetInsertSortedUnguardedLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetInsertSortedUnguarded_INT64_C32)(benchmark::State& state) {
	SetInsertSortedUnguardedLoop(btree_set<std::int64_t, 32>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetInsertSortedUnguardedLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}

#define _BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fix, TestName) _BENCHMARK_REGISTER_SORTED_UNGUARDED_F(Fix, TestName, benchmark::kNanosecond);

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SetInsertSortedUnguarded_INT64)

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, BtreeSetInsertSortedUnguarded_INT64_C32)

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_INSERT_SORTED_UNGUARDED(Fixture, SegmentedSetInsertSortedUnguarded_BIG_BINARY_INT64_C8192)


#endif // INSERT_SORTED_TEST




#if ERASE_RANGE

template<typename C>
inline
std::pair<str2d::Iterator<C>, str2d::Iterator<C>> SetEraseRange(C& set, str2d::Iterator<C> first, str2d::Iterator<C> last, benchmark::State& state) {
	first = set.erase(first, last);
	state.PauseTiming();
	last = SetInsertSorted(set, first, Fixture::single_insert_value.begin(), Fixture::single_insert_value.begin() + state.range(1));
	first = str2d::flat::predecessor(last, state.range(1));
	state.ResumeTiming();
	return { first, last };
}

template<typename C>
inline
std::pair<str2d::Iterator<C>, str2d::Iterator<C>> BtreeSetEraseRange(C& set, str2d::Iterator<C> first, str2d::Iterator<C> last, benchmark::State& state) {
	set.erase(first, last);
	state.PauseTiming();
	last = SetInsertSorted(set, set.begin(), Fixture::single_insert_value.begin(), Fixture::single_insert_value.begin() + state.range(1));
	first = str2d::flat::predecessor(last, state.range(1));
	state.ResumeTiming();
	return { first, last };
}


template<typename C>
inline
std::pair<str2d::Iterator<C>, str2d::Iterator<C>> SegmentedSetEraseRange(C& set, str2d::Iterator<C> first, str2d::Iterator<C> last, benchmark::State& state) {
	first = set.erase(first, last);
	state.PauseTiming();
	std::pair<str2d::Iterator<C>, str2d::Iterator<C>> r = set.insert_sorted_unguarded(first, Fixture::single_insert_value.begin(), str2d::SizeType<C>(state.range(1)));
	state.ResumeTiming();
	return r;
}

template<typename C>
inline
std::pair<str2d::Iterator<C>, str2d::Iterator<C>> Init(C& set, benchmark::State& state) {
	ConstructSetFromSorted(set, state.range(0));
	str2d::Iterator<C> it = set.upper_bound(Fixture::single_insert_value.front());
	str2d::IteratorDifferenceType<str2d::Iterator<C>> diff = std::distance(set.begin(), it);
	SetInsertSorted(set, it, Fixture::single_insert_value.begin(), Fixture::single_insert_value.begin() + state.range(1));

	str2d::Iterator<C> first = str2d::flat::successor(set.begin(), diff);
	return { first, str2d::flat::successor(first, state.range(1)) };
}

template<typename C>
inline
void SetEraseRangeLoop(C& set, benchmark::State& state) {
	auto [first, last] = Init(set, state);

	for (auto _ : state) {
		std::pair<str2d::Iterator<C>, str2d::Iterator<C>> r = SetEraseRange(set, first, last, state);
		first = r.first;
		last = r.second;
	}
}

template<typename C>
inline
void BtreeSetEraseRangeLoop(C& set, benchmark::State& state) {
	auto [first, last] = Init(set, state);

	for (auto _ : state) {
		std::pair<str2d::Iterator<C>, str2d::Iterator<C>> r = BtreeSetEraseRange(set, first, last, state);
		first = r.first;
		last = r.second;
	}
}

template<typename C>
inline
void SegmentedSetEraseRangeLoop(C& set, benchmark::State& state) {
	ConstructSegmentedSetFromSorted(set, state.range(0));
	str2d::Iterator<C> it = set.upper_bound(Fixture::single_insert_value.front());
	str2d::IteratorDifferenceType<str2d::Iterator<C>> diff = str2d::seg::distance(set.begin(), it);
	set.insert_sorted_unguarded(it, Fixture::single_insert_value.begin(), Fixture::max_single_value_nm);

	str2d::Iterator<C> first = str2d::seg::successor(set.begin(), diff);
	str2d::Iterator<C> last = str2d::seg::successor(first, state.range(1));

	for (auto _ : state) {
		std::pair<str2d::Iterator<C>, str2d::Iterator<C>> r = SegmentedSetEraseRange(set, first, last, state);
		first = r.first;
		last = r.second;
	}
}



BENCHMARK_DEFINE_F(Fixture, SetEraseRange_INT64)(benchmark::State& state) {
	SetEraseRangeLoop(std::multiset<std::int64_t>(), state);
}

BENCHMARK_DEFINE_F(Fixture, BtreeSetEraseRange_INT64_C32)(benchmark::State& state) {
	BtreeSetEraseRangeLoop(btree_set<std::int64_t, 32>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_small_binary<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_linear<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_linear<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_linear<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_linear<std::int64_t, 8192>(), state);
}

BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C1024)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_binary<std::int64_t, 1024>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C2048)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_binary<std::int64_t, 2048>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C4096)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_binary<std::int64_t, 4096>(), state);
}
BENCHMARK_DEFINE_F(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C8192)(benchmark::State& state) {
	SegmentedSetEraseRangeLoop(segmented_set_big_binary<std::int64_t, 8192>(), state);
}



#define _BENCHMARK_REGISTER_F_ERASE_RANGE(Fix, TestName) _BENCHMARK_REGISTER_SORTED_UNGUARDED_F(Fix, TestName, benchmark::kNanosecond);

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SetEraseRange_INT64)

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, BtreeSetEraseRange_INT64_C32)

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_SMALL_BINARY_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C2048) 
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_LINEAR_INT64_C8192)

_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C1024)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C2048)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C4096)
_BENCHMARK_REGISTER_F_ERASE_RANGE(Fixture, SegmentedSetEraseRange_BIG_BINARY_INT64_C8192)

#endif


BENCHMARK_MAIN();

*/