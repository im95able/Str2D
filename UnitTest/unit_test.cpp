#include "compile_options.h"

#ifdef SEG_TEST

#include <tuple>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <cmath>
#include <random>
#include <chrono>
#include <cstddef>
#include <numeric>

#include "gtest/gtest.h"

#include "..\Segmented-Map\seg_algorithm.h"
#include "..\Segmented-Map\seg_container.h"


namespace str2d
{ 

namespace seg
{

namespace test
{

#ifdef SEG_POD_TEST

using value_type = int;

inline
int value_of(value_type v) {
	return v;
}

#else

#include "instrumented.h"

using value_type = instrumented<int>;

inline
int value_of(value_type v) {
	return v.value;
}


#endif // SEG_POD_TEST

static constexpr size_t capacity = 100;

#ifdef BIG_HEADER
using header = big_segment_header<value_type, capacity>;
#else
using header = small_segment_header<value_type, capacity>;
#endif

using area = AreaType<header>;

struct allocator_base
{
	enum operations
	{
		allocation = 0,
		deallocation,
		number_ops
	};
	static size_t counts[number_ops];
	static const char* counter_names[number_ops];
	static void init() {
		std::fill(std::begin(counts), std::end(counts), 0);
	}
};

size_t allocator_base::counts[number_ops];
const char* allocator_base::counter_names[number_ops] = {
	"allocation",
	"deallocation",
};

#ifdef POOL_ALLOCATOR_TEST

#include "..\Segmented-Map\pool_allocator.h"

static constexpr std::size_t chunk_capacity = 100;

struct allocator : allocator_base
{
	using alloc_type = str2d::pool_allocator<area, chunk_capacity>;
	using value_type = typename alloc_type::value_type;
	using size_type = typename alloc_type::size_type;
	using difference_type = typename alloc_type::difference_type;
	using is_always_equal = typename alloc_type::is_always_equal;

	alloc_type alloc;

	allocator(std::size_t reserve_chunks = 0) : alloc(reserve_chunks) {}

	template<typename O>
	struct rebind {
		using other = allocator;
	};

	value_type* allocate(size_t n) {
		++counts[allocation];
		return alloc.allocate(n);
	}
	void deallocate(value_type* a, size_t n) {
		++counts[deallocation];
		return alloc.deallocate(a, n);
	}
};

#else

struct allocator : allocator_base
{
	std::allocator<area> alloc;

	area* allocate(size_t n) {
		++counts[allocation];
		return alloc.allocate(n);
	}
	void deallocate(area* a, size_t n) {
		++counts[deallocation];
		return alloc.deallocate(a, n);
	}
};

#endif

#ifdef BIG_HEADER
using vector = seg::vector_big_header<value_type, capacity, allocator>;
#else
using vector = seg::vector_small_header<value_type, capacity, allocator>;
#endif

using index = typename vector::index;
using header_iterator = Iterator<index>;
using const_header_iterator = ConstIterator<index>;
using multiset = seg::multiset_tmp<value_type, std::less<value_type>, vector, flat::find_adaptor_linear, flat::equal_range_adaptor_linear>;
using const_segmented_coordinate = typename multiset::const_segmented_coordinate;
using segmented_coordinate = typename multiset::segmented_coordinate;
using segment_iterator = typename segmented_coordinate::segment_iterator;
using const_segment_iterator = typename segmented_coordinate::const_segment_iterator;
using flat_iterator = typename segment_iterator::flat_iterator;
using iterator = typename std::vector<value_type>::iterator;
using const_iterator = typename std::vector<value_type>::const_iterator;

static std::default_random_engine rand_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

std::pair<size_t, size_t> rand_begin_and_end(size_t c) {
	std::uniform_int_distribution<size_t> uniform_size(limit(c), c);
	size_t s = uniform_size(rand_engine);
	std::uniform_int_distribution<size_t> uniform_first(0, c - s);
	size_t first = uniform_first(rand_engine);
	return { first, first + s };
}

std::pair<size_t, size_t> _rand_begin_and_end_not_full(size_t min, size_t c) {
	std::uniform_int_distribution<size_t> uniform_size(min, c - 1);
	size_t s = uniform_size(rand_engine);
	std::uniform_int_distribution<size_t> uniform_first(0, c - s);
	size_t first = uniform_first(rand_engine);
	return { first, first + s };
}

std::pair<size_t, size_t> rand_begin_and_end_not_full(size_t c) {
	return _rand_begin_and_end_not_full(limit(c), c);
}

std::pair<size_t, size_t> rand_begin_and_end_not_full_edge(size_t c) {
	return _rand_begin_and_end_not_full(1, c);
}

size_t rand(size_t min, size_t max) {
	return std::uniform_int_distribution<size_t>(min, max)(rand_engine);
}

size_t rand(size_t max) {
	return std::uniform_int_distribution<size_t>(0, max)(rand_engine);
}

std::tuple<size_t, size_t, size_t> rand_n0en1(size_t n) {
	size_t n0 = rand(n >> 1);
	size_t e = rand(n - n0);
	return { n0, e, n - (n0 + e) };
}

void destroy_index(index& in) {
	segment_iterator first_seg(in.begin());
	segment_iterator last_seg(in.end());
	seg::destruct(first_seg, std::begin(first_seg), last_seg, std::begin(last_seg));
	in.erase(in.begin(), in.end());
}

struct equal_headers
{
	bool operator()(const header& h0, const header& h1) {
		return 
			seg::area(h0) == seg::area(h1) &&
			seg::begin_index(h0) == seg::begin_index(h1) &&
			seg::end_index(h0) == seg::end_index(h1);
	}
};

template<typename T0, typename T1, typename T2 = T0>
void check_equal(const T0& s0, const T1& s1, const char* message_smaller, const char* message_larger) {
	ASSERT_LE(static_cast<T2>(s0), static_cast<T2>(s1)) << message_larger;
	ASSERT_GE(static_cast<T2>(s0), static_cast<T2>(s1)) << message_smaller;
}

size_t create_rand_range(header_iterator first_seg, size_t n) {
	size_t final_size = 0;
	int i = 0;
	while (n) {
		size_t s = rand(seg::limit(*first_seg), seg::capacity(*first_seg));
		size_t f = rand(seg::capacity(*first_seg) - s);
		seg::set_begin_index(*first_seg, f);
		seg::set_end_index(*first_seg, f + s);
		value_type * first = seg::begin(*first_seg);
		final_size = final_size + s;
		while (s) {
			construct_at(first, i);
			++i;
			++first;
			--s;
		}
		++first_seg;
		--n;
	}
	return final_size;
}

class area_allocator {
	static std::allocator<area> alloc;

public:
	static area* allocate() {
		return alloc.allocate(1);
	}

	static void deallocate(area* data) {
		alloc.deallocate(data, 1);
	}
};

std::allocator<area> area_allocator::alloc;

inline
void init_range(value_type* first, value_type* last, int i) {
	while (first != last)
		construct_at(first++, i++);
}

void create_segment(header& h, size_t first_index, size_t last_index) {
	seg::set_begin_index(h, first_index);
	seg::set_end_index(h, last_index);
	value_type* first = seg::begin(h);
	value_type* last = seg::end(h);
	init_range(first, last, 0);
}

void create_segment_with_allocation(header& h, size_t first_index, size_t last_index) {
	seg::set_area(h, area_allocator::allocate());
	create_segment(h, first_index, last_index);
}

void test_segment_size(const header& h, size_t l) {
	ASSERT_LE(begin_index(h), end_index(h)) <<
		"Begin index in not smaller than the end index";

	ASSERT_GE(seg::size(h), l) <<
		"Size of the segment is smaller than the limit";
}

inline
void test_correct_segment_and_range_size(header_iterator first, header_iterator last, size_t size = 0) {
	if (first != last) {
		size_t s = 0;

		ASSERT_LE(first, last) <<
			"First segment of the entire range is after last";

		test_segment_size(*first, 1);
		s = s + seg::size(*first);
		++first;

		while (first != last) {
			test_segment_size(*first, limit(*first));
			s = s + seg::size(*first);
			++first;
		}

		if (size > 0) {
			check_equal(
				s,
				size,
				"Range is smaller than it should be",
				"Range is larger than in should be");
		}
	}
	test_segment_size(*first, 0);
}


inline
void test_segment_balance(const value_type* firstu, const value_type* lastu, const value_type* first, const value_type* last) {
	std::int64_t front = first - firstu;
	std::int64_t back = lastu - last;
	std::int64_t diff = std::abs(front - back);

	ASSERT_TRUE(diff == 0 || diff == 1) <<
		"Front and back ranges are not equal in size";
}

inline
void test_segment_balance(const header& h) {
	test_segment_balance(data(h), flat::successor(data(h), seg::capacity(h)), seg::begin(h), seg::end(h));
}

inline
void test_per_segment_balance(header_iterator first, header_iterator last) {
	while (first != last) test_segment_balance(*first++);
}

void destroy_segment(header& h) {
	if (h.area) {
		flat::destruct(seg::begin(h), seg::end(h));
		area_allocator::deallocate(seg::area(h));
	}
}


void check_construction_and_destruction() {
#ifndef SEG_POD_TEST
	check_equal(
		instrumented_base::counts[instrumented_base::constructor],
		instrumented_base::counts[instrumented_base::destructor],
		"More objects were destructed than constructed",
		"More objects were constructed than destructed"
	);
#endif
}

void check_allocation_and_deallocation() {
	check_equal(
		allocator_base::counts[allocator_base::allocation],
		allocator_base::counts[allocator_base::deallocation],
		"More areas were deallocated than allocated",
		"More areas were allocated than deallocated"
	);
}

struct InternalTestBase : public testing::Test
{
	virtual void SetUpSeg() {}
	virtual void TearDownSeg() {}

	void SetUp() override {
		allocator_base::init();

#ifndef SEG_POD_TEST
		instrumented_base::init();
#endif

		SetUpSeg();
	}

	void TearDown() override {
		TearDownSeg();

		check_allocation_and_deallocation();
		check_construction_and_destruction();
	}
};

#ifdef INTERNAL_FLAT_INSERT_TEST
struct TestInsertFlat : public InternalTestBase
{
	using iterator = Iterator<std::vector<value_type>>;

	static std::vector<value_type> v0;
	static std::vector<value_type> v1;

	iterator first_init;
	iterator last_init; 
	iterator _first_init; // After insert first
	iterator _last_init; // After insert last
	size_t insert_at;
	size_t insert_nm;

	void TearDownSeg() override {
		v0.clear();
		v1.clear();
	}

	void InitAndInsert(
		size_t _first_init_index,
		size_t _last_init_index,
		size_t _insert_at,
		size_t _insert_nm) {

		v0.resize(capacity);
	
		int init_size = static_cast<int>(_last_init_index - _first_init_index);
		first_init = v0.begin() + _first_init_index;
		last_init = v0.begin() + _last_init_index;
		insert_at = _insert_at;
		insert_nm = _insert_nm;
		std::fill(v0.begin(), first_init, value_type(capacity));
		std::fill(last_init, v0.end(), value_type(capacity));
		for (int i = 0; i < init_size; ++i) *(first_init + i) = value_type(i);

		v1 = std::vector(first_init, last_init);
		std::tie(_first_init, _last_init) = insert_flat(v0.begin(), v0.end(), first_init, last_init, first_init + insert_at, insert_nm);

		_TestRangeRemainsEqual();
	}

	void _TestRangeRemainsEqual() {
		ASSERT_EQ(_last_init - _first_init, v1.size() + insert_nm) <<
			"Range is not the correct size";

		ASSERT_TRUE(std::equal(v1.begin(), v1.begin() + insert_at, _first_init, _first_init + insert_at)) <<
			"Pre range is not the same as before insertion";

		ASSERT_TRUE(std::equal(v1.begin() + insert_at, v1.end(), _last_init - ((last_init - first_init) - insert_at), _last_init)) <<
			"Post range is not the same as before insertion";
	}
};

std::vector<value_type> TestInsertFlat::v0;
std::vector<value_type> TestInsertFlat::v1;

TEST_F(TestInsertFlat, PostSmallerBackAvailable)
{
	size_t last_init_index = rand(capacity - (capacity / 3), capacity - (capacity / 5));
	size_t back_size = capacity - last_init_index;
	size_t first_init_index = rand(0, back_size >> 1);
	size_t size = last_init_index - first_init_index;
	size_t insert_at = rand(size - (size >> 2), size);
	size_t insert_nm = rand(first_init_index + 1, back_size);

	InitAndInsert(first_init_index, last_init_index, insert_at, insert_nm);

	ASSERT_EQ(_first_init, first_init) <<
		"Pre range was moved forward even though post range is smaller and there is enough space in the back";

	ASSERT_EQ(_last_init, last_init + insert_nm) <<
		"Post range was not moved into the right position";
}

TEST_F(TestInsertFlat, PreSmallerFrontAvailable)
{
	size_t first_init_index = rand(capacity / 5, capacity / 3);
	size_t last_init_index = rand(capacity - (first_init_index >> 1), capacity);
	size_t back_size = capacity - last_init_index;
	size_t size = last_init_index - first_init_index;
	size_t insert_at = rand(0, size >> 2);
	size_t insert_nm = rand(back_size + 1, first_init_index);

	InitAndInsert(first_init_index, last_init_index, insert_at, insert_nm);

	ASSERT_EQ(_last_init, last_init) <<
		"Post range was moved backward even though there isn't enough available space in back";

	ASSERT_EQ(_first_init, first_init - insert_nm) <<
		"Pre range was not moved into the right position";
}

TEST_F(TestInsertFlat, PreSmallerBackAvailable)
{
	size_t last_init_index = rand(capacity - (capacity / 3), capacity - (capacity / 5));
	size_t back_size = capacity - last_init_index;
	size_t first_init_index = rand(0, back_size >> 1);
	size_t size = last_init_index - first_init_index;
	size_t insert_at = rand(0, size >> 2);
	size_t insert_nm = rand(first_init_index + 1, back_size);

	InitAndInsert(first_init_index, last_init_index, insert_at, insert_nm);

	ASSERT_EQ(_first_init, first_init) <<
		"Pre range was moved forward even though there isn't enough available space in front";

	ASSERT_EQ(_last_init, last_init + insert_nm) <<
		"Post range was not moved into the right position";
}

TEST_F(TestInsertFlat, PostSmallerFrontAvailable)
{
	size_t first_init_index = rand(capacity / 5, capacity / 3);
	size_t last_init_index = rand(capacity - (first_init_index >> 1), capacity);
	size_t back_size = capacity - last_init_index;
	size_t size = last_init_index - first_init_index;
	size_t insert_at = rand(size - (size >> 2), size);
	size_t insert_nm = rand(back_size + 1, first_init_index);

	InitAndInsert(first_init_index, last_init_index, insert_at, insert_nm);

	ASSERT_EQ(_last_init, last_init) <<
		"Post range was moved backward even though pre range is smaller and there is enough space in the front";

	ASSERT_EQ(_first_init, first_init - insert_nm) <<
		"Pre range was not moved into the right position";
}

TEST_F(TestInsertFlat, FrontBackAvailableCombined)
{
	size_t first_init_index = rand(capacity / 5, capacity / 4);
	size_t last_init_index = capacity - rand(capacity / 5, capacity / 4);
	size_t back_size = capacity - last_init_index;
	size_t size = last_init_index - first_init_index;
	size_t insert_at = rand(0, size);
	size_t insert_nm = rand(std::max(first_init_index, back_size) + 1, capacity - size);

	InitAndInsert(first_init_index, last_init_index, insert_at, insert_nm);

	ASSERT_NE(_last_init, last_init) <<
		"Pre range wasn't moved backward even though there is enough space only if both pre and post ranges get moved";

	ASSERT_NE(_first_init, first_init) <<
		"Post range wasn't moved forward even though there is enough space only if both pre and post ranges get moved";

	ASSERT_EQ(_last_init - _first_init, (last_init - first_init) + insert_nm) <<
		"New range doesn't have the correct size";
}
#endif // INTERNAL_FLAT_INSERT_TEST




#ifdef INTERNAL_FLAT_ERASE_TEST

struct TestEraseFlat : public InternalTestBase
{
	static std::vector<value_type> v0;
	static std::vector<value_type> v1;

	void TearDownSeg() override {
		v0.clear();
		v1.clear();
	}

	size_t erase_nm;
	size_t erase_at;
	iterator first;
	iterator last;
	iterator erase_pos;

	void InitAndErase(
		size_t _first_init_index,
		size_t _last_init_index,
		size_t _erase_at,
		size_t _erase_nm) {

		erase_nm = _erase_nm;
		erase_at = _erase_at;
		int init_size = static_cast<int>(_last_init_index - _first_init_index);
		v0.resize(init_size);
		first = v0.begin();
		for (int i = 0; i < init_size; ++i) *(first + i) = value_type(i);
		v1 = std::vector(first, v0.end());
		iterator firste = first + _erase_at;
		iterator laste = firste + _erase_nm;
		std::tie(first, last, erase_pos) = seg::erase_flat(first, v0.end(), firste, laste);
	}

	void TestRangeRemainsEqual() {
		check_equal(
			last - first,
			v1.size() - erase_nm,
			"Range is smaller than it should be",
			"Range is larger than it shuold be");

		check_equal(
			erase_pos - first,
			erase_at,
			"Pre range is smaller than it should be",
			"Pre range is larger than it should be");

		size_t back = v1.size() - (erase_at + erase_nm);
		check_equal(
			last - erase_pos,
			back,
			"Post range is smaller than it should be",
			"Post range is larger than it should be");

		ASSERT_TRUE(std::equal(first, erase_pos, v1.begin())) <<
			"Pre range is not same as before erasure";

		ASSERT_TRUE(std::equal(erase_pos, last, v1.end() - back)) <<
			"Post range is not same as before erasure";
	}
};

std::vector<value_type> TestEraseFlat::v0;
std::vector<value_type> TestEraseFlat::v1;

TEST_F(TestEraseFlat, Erase)
{
	auto [first_index, last_index] = rand_begin_and_end_not_full(capacity);
	size_t s = last_index - first_index;
	size_t erase_nm = rand(s - (s / 3));
	size_t erase_at = rand(s - erase_nm);
	InitAndErase(first_index, last_index, erase_at, erase_nm);
	TestRangeRemainsEqual();
	if (erase_at < s - (erase_at + erase_nm)) {
		ASSERT_EQ(first - v0.begin(), erase_nm) <<
			"Pre range was moved backward even though it's smaller than post range";
	}
	else {
		ASSERT_EQ(v0.end() - last, erase_nm) <<
			"Post range was moved forward even though it's smaller than pre range";
	}
}

#endif // INTERNAL_FLAT_ERASE_TEST



#ifdef INTERNAL_SEGMENT_SLIDE_TEST

struct TestSegmentSlide : public InternalTestBase
{
	header h;
	static std::vector<value_type> v;

	void InitAndSlide(
		size_t _first_init_index,
		size_t _last_init_index,
		size_t _move_to_index) {

		create_segment_with_allocation(h, _first_init_index, _last_init_index);
		v = std::vector<value_type>(seg::begin(h), seg::end(h));
		if (_move_to_index > _first_init_index) 
			seg::slide_segment_backward(h, _move_to_index - _first_init_index);
		else 
			seg::slide_segment(h, _first_init_index - _move_to_index);

	}

	void TearDownSeg() override {
		destroy_segment(h);
		v.clear();
	}

	void TestRangesRemainEqual() {
		ASSERT_LE(seg::end(h), seg::data(h) + seg::capacity(h)) <<
			"Range was moved past the end of segment data";

		check_equal(
			seg::size(h),
			v.size(),
			"Range is smaller than it should be",
			"Range is larger than it should be");

		ASSERT_TRUE(std::equal(v.begin(), v.end(), seg::begin(h))) <<
			"Range is no longer the same";
	}

};

std::vector<value_type> TestSegmentSlide::v;

TEST_F(TestSegmentSlide, SlideForward)
{
	auto [first_index, last_index] = rand_begin_and_end_not_full(capacity);
	size_t size = last_index - first_index;
	InitAndSlide(first_index, last_index, rand(0, first_index));
	TestRangesRemainEqual();
}


TEST_F(TestSegmentSlide, SlideBackward)
{
	auto [first_index, last_index] = rand_begin_and_end_not_full(capacity);
	size_t size = last_index - first_index;
	InitAndSlide(first_index, last_index, rand(first_index, capacity - size));
	TestRangesRemainEqual();
}


#endif // INTERNAL_SEGMENT_SLIDE_TEST


#ifdef INTERNAL_SEGMENT_MOVE_TEST


struct TestSegmentMove : public InternalTestBase
{
	static std::vector<value_type> v0;
	static std::vector<value_type> v1;

	header h0;
	header h1;

	size_t n0;
	size_t e;
	size_t n1;
	size_t right_last;
	size_t left_first;

	void TearDownSeg() override {
		v0.clear();
		v1.clear();
		destroy_segment(h0);
		destroy_segment(h1);
	}

	void _InitHeaderAndContainer(header& h, std::vector<value_type>& v, size_t _first_init_index, size_t _last_init_index) {
		create_segment_with_allocation(h, _first_init_index, _last_init_index);
		v = std::vector<value_type>(seg::begin(h), seg::end(h));
	}

	void InitLeft(size_t _first_init_index, size_t _last_init_index) {
		left_first = _first_init_index;
		_InitHeaderAndContainer(h0, v0, _first_init_index, _last_init_index);
		

	}
	void InitRight(size_t _first_init_index, size_t _last_init_index) {
		right_last = _last_init_index;
		_InitHeaderAndContainer(h1, v1, _first_init_index, _last_init_index);

	}

	void MoveToRight(size_t _n0, size_t _e, size_t _n1) {
		n0 = _n0;
		e = _e;
		n1 = _n1;
		seg::move_to_right(h0, h1, n0, e, n1);
		init_range(seg::begin(h1) + n1, seg::begin(h1) + n1 + e, 0);
	}

	void MoveToRight(size_t _n) {
		n0 = _n;
		e = 0;
		n1 = 0;
		seg::move_to_right(h0, h1, n0);
	}

	void MoveToLeft(size_t _n0, size_t _e, size_t _n1) {
		n0 = _n0;
		e = _e;
		n1 = _n1;
		seg::move_to_left(h1, h0, n0, e, n1);
		init_range(seg::end(h0) - n1 - e, seg::end(h0) - n1, capacity);
	}

	void MoveToLeft(size_t _n) {
		n0 = _n;
		e = 0;
		n1 = 0;
		seg::move_to_left(h1, h0, n0);
	}

	void MoveToRightEmpty(size_t _f, size_t _n) {
		n0 = _n;
		e = 0;
		n1 = 0;
		seg::move_to_right_empty(h0, h1, _f, n0);
	}

	void TestRightSegmentBalance() {
		test_segment_balance(h1);
	}

	void TestLeftSegmentBalance() {
		test_segment_balance(h0);
	}

	void _TestRangeRemainsEqual() {
		check_equal(
			seg::size(h0) + seg::size(h1),
			v0.size() + v1.size() + e,
			"Number of elements is smaller than before",
			"Number of elements is larger than before");
	}

	void TestRangeRemainsEqualAfterMoveToLeft() {
		_TestRangeRemainsEqual();

		ASSERT_TRUE(std::equal(v0.begin(), v0.end(), seg::begin(h0))) <<
			"Elements on the left segment are no longer the same";

		auto v1_n0 = v1.begin() + n0;
		auto v1_n1 = v1_n0 + n1;

		ASSERT_TRUE(std::equal(v1.begin(), v1_n0, seg::begin(h0) + v0.size())) <<
			"Elements(n0) which were moved to the left segment are no longer the same";

		ASSERT_TRUE(std::equal(v1_n0, v1_n1, seg::begin(h0) + v0.size() + n0 + e)) <<
			"Elements(n1) which were moved to the left segment are no longer the same";

		ASSERT_TRUE(std::equal(v1_n1, v1.end(), seg::begin(h1))) <<
			"Elements which remain on the right segment are no longer the same";

	}

	void TestRangeRemainsEqualAfterMoveToRight() {
		_TestRangeRemainsEqual();

		ASSERT_TRUE(std::equal(seg::begin(h0), seg::end(h0), v0.begin())) <<
			"Elements on which remain on the left segment are no longer the same";

		auto v0_n1 = v0.begin() + seg::size(h0) + n1;
		auto v0_n0 = v0_n1 + n0;

		ASSERT_TRUE(std::equal(v0.begin() + seg::size(h0), v0_n1, seg::begin(h1))) <<
			"Elements(n1) which were moved to the left segment are no longer the same";

		ASSERT_TRUE(std::equal(v0_n1, v0_n0, seg::begin(h1) + n1 + e)) <<
			"Elements(n0) which were moved to the left segment are no longer the same";

		ASSERT_TRUE(std::equal(v1.begin(), v1.end(), seg::begin(h1) + n1 + e + n0)) <<
			"Elements on right are no longer the same";
	}

	void TestLeftRangeNotMoved() {
		ASSERT_EQ(left_first, begin_index(h0)) <<
			"Left range was moved even though there was enough space in back";
	}

	void TestRightRangeNotMoved() {
		ASSERT_EQ(right_last, end_index(h1)) <<
			"Right range was moved even though there was enough space in front";
	}
};

std::vector<value_type> TestSegmentMove::v0;
std::vector<value_type> TestSegmentMove::v1;

TEST_F(TestSegmentMove, MoveToRightN)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	InitRight(right_first, right_last);
	size_t move = rand(capacity - (right_last - right_first));
	if (move > left_last - left_first)
		move = rand(left_last - left_first);
	MoveToRight(move);
	TestRangeRemainsEqualAfterMoveToRight();
	if (move > right_first) {
		TestRightSegmentBalance();
	}
	else {
		TestRightRangeNotMoved();
	}
}

TEST_F(TestSegmentMove, MoveToRightN0EN1)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	InitRight(right_first, right_last);
	size_t right_available = rand(capacity - (right_last - right_first));
	size_t move = rand(right_available);
	if (move > left_last - left_first)
		move = rand(left_last - left_first);

	auto [n0, e, n1] = rand_n0en1(move);
	MoveToRight(n0, e, n1);
	TestRangeRemainsEqualAfterMoveToRight();
	if (move > right_first) {
		TestRightSegmentBalance();
	}
	else {
		TestRightRangeNotMoved();
	}
}

TEST_F(TestSegmentMove, MoveToLeftN)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	InitRight(right_first, right_last);
	size_t move = rand(capacity - (left_last - left_first));
	if (move > right_last - right_first)
		move = rand(right_last - right_first);
	MoveToLeft(move);
	TestRangeRemainsEqualAfterMoveToLeft();
	if (move > capacity - left_last) {
		TestLeftSegmentBalance();
	}
	else {
		TestLeftRangeNotMoved();
	}
}

TEST_F(TestSegmentMove, MoveToLeftN0EN1)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	InitRight(right_first, right_last);
	size_t left_available = rand(capacity - (left_last - left_first));
	size_t move = rand(left_available);
	if (move > right_last - right_first)
		move = rand(right_last - right_first);

	auto [n0, e, n1] = rand_n0en1(move);
	MoveToLeft(n0, e, n1);
	TestRangeRemainsEqualAfterMoveToLeft();
	if (move > capacity - left_last) {
		TestLeftSegmentBalance();
	}
	else {
		TestLeftRangeNotMoved();
	}
}

TEST_F(TestSegmentMove, MoveToRightEmpty)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	InitRight(capacity, capacity);
	size_t left_size = left_last - left_first;
	size_t s = rand(left_size);
	size_t insert_at = rand(capacity - s);
	MoveToRightEmpty(insert_at, s);
	TestRangeRemainsEqualAfterMoveToRight();

	ASSERT_EQ(seg::begin_index(h1), insert_at) <<
		"First index of right was not set correctly";
}


#endif // INTERNAL_SEGMENT_MOVE_TEST



#ifdef INTERNAL_INDEX_TEST

struct TestIndex : public InternalTestBase
{
	static index in;
	static std::vector<header> v;

	void TearDownSeg() override {
		destroy_index(in);
		v.clear();
	}

	void Init(size_t n) {
		header_iterator it = in.insert(in.begin(), n);

		ASSERT_EQ(it, in.begin()) <<
			"Begin iterator of the inserted range is not the begin iterator od the index";

		ASSERT_EQ(in.size(), n) <<
			"Size of the index after the initial insert is not correct";

		while (n) seg::set_begin_end_indices(*it++, n--);
		
		v.resize(in.size() + 1);
		flat::copy_n(in.begin(), in.size() + 1, v.begin());
	}
};

TEST_F(TestIndex, Insert) {
	size_t initial_size = rand(10, 20);
	size_t insert_nm = rand(40);
	size_t insert_at = rand(initial_size);
	Init(initial_size);

	header_iterator it = in.insert(in.begin() + insert_at, insert_nm);

	check_equal(
		in.size(),
		initial_size + insert_nm,
		"Size of the index is smaller than it should be",
		"Size of the index is larger than it should be");

	ASSERT_EQ(it - in.begin(), insert_at) <<
		"Inserted range is not placed into the right position";

	ASSERT_TRUE(std::equal(in.begin(), it, v.begin(), equal_headers())) <<
		"Pre headers are no longer the same";

	ASSERT_TRUE(std::equal(it + insert_nm, in.end() + 1, v.begin() + insert_at, equal_headers())) <<
		"Post headers are no longer the same";
}

TEST_F(TestIndex, Erase) {

	size_t initial_size = rand(10, 20);
	size_t erase_nm = rand(initial_size);
	size_t erase_at = rand(initial_size - erase_nm);
	Init(initial_size);

	header_iterator it = in.erase(in.begin() + erase_at, in.begin() + erase_at + erase_nm);

	check_equal(
		in.size(),
		initial_size - erase_nm,
		"Size of the index is smaller than it should be",
		"Size of the index is larger than it should be");

	ASSERT_EQ(it - in.begin(), erase_at) <<
		"Inserted range is not placed into the right position";

	ASSERT_TRUE(std::equal(in.begin(), it, v.begin(), equal_headers())) <<
		"Pre headers are no longer the same";

	ASSERT_TRUE(std::equal(it, in.end() + 1, v.begin() + erase_at + erase_nm, equal_headers())) <<
		"Post headers are no longer the same";
}

index TestIndex::in;
std::vector<header> TestIndex::v;

#endif // INTERNAL_INDEX_TEST


#define MEMORY_OVERFLOW_BUG


#if defined(INTERNAL_SEGMENTED_INSERT_TEST) || defined(INTERNAL_SEGMENTED_ERASE_TEST)

struct TestSegmentRangeBase : public InternalTestBase
{
	static std::vector<value_type> v;
	static index in;
	static segmented_coordinate first_inserted;
	static segmented_coordinate last_inserted;

	void SetUpSeg() override {
		first_inserted = begin();
		last_inserted = begin();
	}

	void TearDownSeg() override {
		seg::destruct(begin(), first_inserted);
		seg::destruct(last_inserted, end());
		in.clear();
		v.clear();
	}

	segmented_coordinate begin() { return segmented_coordinate(in.begin(), seg::begin(*in.begin())); }
	segmented_coordinate end() { return segmented_coordinate(in.end(), seg::begin(*in.end())); }
	header_iterator left() { return in.begin(); }
	header_iterator right() { return --in.end(); }

	size_t InitRandRange(size_t nm_segments) {
		in.insert(in.begin(), static_cast<SizeType<index>>(nm_segments));
		size_t s = create_rand_range(in.begin(), nm_segments);
		v.resize(static_cast<SizeType<std::vector<value_type>>>(s));
		seg::copy_seg_flat(begin(), end(), v.begin());
		return s;
	}

	void InitLeft(size_t _first_init_index, size_t _last_init_index) {
		header_iterator it = in.insert(in.begin(), 1);
		create_segment(*it, _first_init_index, _last_init_index);
		v.insert(v.begin(), seg::begin(*it), seg::end(*it));
	}

	void InitRight(size_t _first_init_index, size_t _last_init_index) {
		header_iterator it = in.insert(in.end(), 1);
		create_segment(*it, _first_init_index, _last_init_index);
		v.insert(v.end(), seg::begin(*it), seg::end(*it));
	}

	void InitLeft(std::pair<size_t, size_t> p) {
		InitLeft(p.first, p.second);
	}

	void InitRight(std::pair<size_t, size_t> p) {
		InitRight(p.first, p.second);
	}
};

index TestSegmentRangeBase::in;
std::vector<value_type> TestSegmentRangeBase::v;
segmented_coordinate TestSegmentRangeBase::first_inserted;
segmented_coordinate TestSegmentRangeBase::last_inserted;

#endif 



#ifdef INTERNAL_SEGMENTED_INSERT_TEST 


struct TestSegmentInsert : public TestSegmentRangeBase
{
	void TestRangeInsertedCorrectly(
		segmented_coordinate first,
		segmented_coordinate last,
		size_t insert_nm,
		header_iterator first_balance,
		header_iterator last_balance) {

		first_inserted = first;
		last_inserted = last;
		test_correct_segment_and_range_size(in.begin(), in.end(), insert_nm + v.size());
		test_per_segment_balance(first_balance, last_balance);

		ASSERT_LE(first, last) <<
			"First element of the inserted rage is after last";

		check_equal(
			seg::distance(first, last),
			insert_nm,
			"Inserted range is smaller than it should be",
			"Inserted range is larger than it should be");

		ASSERT_LE(begin(), first) <<
			"First element of the inserted range is before the first element of the entire range";
		ASSERT_GE(end(), last) <<
			"Last element of the inserted range is after the last element of the entire range";

		size_t left_size = seg::distance(begin(), first);

		ASSERT_TRUE(seg::equal_flat_n_seg(v.begin(), left_size, begin(), first)) <<
			"Left range is no longer the same";

		ASSERT_TRUE(seg::equal_flat_n_seg(flat::successor(v.begin(), left_size), static_cast<size_t>(v.size()) - left_size, last, end())) <<
			"Right range is no longer the same";
	}

	void TestRangeInsertedCorrectly(
		pair2<header_iterator, size_t> r,
		size_t insert_nm,
		header_iterator first_balance,
		header_iterator last_balance) {

		segment_iterator first_seg = segment_iterator(r.first.first);
		segmented_coordinate first(first_seg, std::begin(first_seg) + r.first.second);
		segment_iterator last_seg = segment_iterator(r.second.first);
		segmented_coordinate last(last_seg, std::begin(last_seg) + r.second.second);

		TestRangeInsertedCorrectly(first, last, insert_nm, first_balance, last_balance);
	}

	void TestRangeInsertedCorrectly(
		pair2<header_iterator, size_t> r,
		size_t insert_nm) {

		TestRangeInsertedCorrectly(r, insert_nm, in.end(), in.end());
	}
};

TEST_F(TestSegmentInsert, InsertCurrentAvailable)
{
	InitRandRange(1);
	size_t left_size = seg::size(*left());
	size_t insert_nm = rand(capacity - left_size);
	size_t insert_at = rand(left_size);

	TestRangeInsertedCorrectly(
		seg::insert_current_available_aux(left(), insert_at, insert_nm),
		insert_nm);
}

TEST_F(TestSegmentInsert, InsertBalanceLeftSimple)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full_edge(capacity);
	InitLeft(left_first, left_last);
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitRight(right_first, right_last);

	size_t left_size = left_last - left_first;
	size_t right_size = right_last - right_first;
	size_t insert_at = rand(right_size);
	size_t available_right = capacity - right_size;
	size_t insert_nm = rand(available_right + 1, available_right + (capacity - left_size));

	TestRangeInsertedCorrectly(
		seg::insert_balance_left_simple(right(), left(), (seg::size(*left()) + seg::size(*right()) + insert_nm) >> 1, insert_at, insert_nm),
		insert_nm);
}

TEST_F(TestSegmentInsert, InsertBalanceRightSimple)
{
	auto [left_first, left_last] = rand_begin_and_end_not_full(capacity);
	InitLeft(left_first, left_last);
	auto [right_first, right_last] = rand_begin_and_end_not_full_edge(capacity);
	InitRight(right_first, right_last);

	size_t left_size = left_last - left_first;
	size_t right_size = right_last - right_first;
	size_t insert_at = rand(left_size);
	size_t available_left = capacity - left_size;
	size_t insert_nm = rand(available_left + 1, available_left + (capacity - right_size));

	TestRangeInsertedCorrectly(
		seg::insert_balance_right_simple(left(), right(), (seg::size(*left()) + seg::size(*right()) + insert_nm) >> 1, insert_at, insert_nm),
		insert_nm);
}

TEST_F(TestSegmentInsert, InsertBalanceLeftIncreaseEmpty)
{
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitRight(right_first, right_last);
	size_t nm_used_segments = rand(2, 10);
	size_t right_size = right_last - right_first;
	size_t max_size = capacity * nm_used_segments;
	size_t min_size = ((nm_used_segments - 1) * capacity) + 1;
	size_t size = rand(min_size, max_size);
	size_t insert_nm = size - right_size;

	auto [nm_segments, m, s] = seg::segment_range_info(capacity, 0, seg::size(*right()), insert_nm);
	header_iterator first = in.insert(right(), nm_segments - 1);
	std::pair<header_iterator, size_t> p = seg::insert_balance_left_increase_empty(first, right(), m, s, insert_nm);
	segment_iterator it(p.first);

	TestRangeInsertedCorrectly(
		begin(), 
		segmented_coordinate(it, std::begin(it) + p.second), 
		insert_nm,
		left(), 
		right());
}

TEST_F(TestSegmentInsert, InsertBalanceLeftIncrease)
{
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitRight(right_first, right_last);
	size_t nm_used_segments = rand(2, 10);
	size_t right_size = right_last - right_first;
	size_t max_size = capacity * nm_used_segments;
	size_t min_size = ((nm_used_segments - 1) * capacity) + 1;
	size_t size = rand(min_size, max_size);
	size_t insert_nm = size - right_size;
	size_t insert_at = rand(right_size);

	auto [nm_segments, m, s] = seg::segment_range_info(capacity, 0, seg::size(*right()), insert_nm);
	size_t left_final_size = m > 0 ? s + 1 : s;
	size_t left_size = rand(1, left_final_size - 1);
	if (left_size >= insert_nm) 
		left_size = insert_nm >> 1;
	insert_nm = insert_nm - left_size;
	size_t left_first = balanced_begin(capacity, left_final_size);
	InitLeft(left_first, left_first + left_size);

	in.insert(right(), nm_segments - 2);
	TestRangeInsertedCorrectly(
		seg::insert_balance_left_increase(left(), right(), m, s, insert_at, insert_nm),
		insert_nm,
		left(),
		right());
}

TEST_F(TestSegmentInsert, InsertBalanceLeft)
{
	auto [right_first, right_last] = rand_begin_and_end_not_full(capacity);
	InitRight(right_first, right_last);

	auto [left_first, left_last] = rand_begin_and_end_not_full_edge(capacity);
	InitLeft(left_first, left_last);

	size_t right_size = right_last - right_first;
	size_t left_size = left_last - left_first;
	size_t init_size = right_size + left_size;
	size_t left_available = capacity - left_size;
	size_t right_available = capacity - right_size;
	size_t nm_used_segments = rand(2, 10);
	size_t max_size = (capacity * nm_used_segments);
	size_t min_size = (nm_used_segments - 1) * capacity + 1;
	size_t size = rand(min_size, max_size);
	if (size < init_size) {
		size = init_size + right_available + (left_available >> 1);
	}
	size_t insert_nm = size - (left_size + right_size);
	if (insert_nm <= right_available) {
		insert_nm = right_available + (left_available >> 1);
	}
	size_t insert_at = rand(right_size);

	auto [nm_segments, m, s] = seg::segment_range_info(capacity, seg::size(*left()), seg::size(*right()), insert_nm);
	header_iterator first = in.insert(right(), nm_segments - 2);

	size_t _insert_at = insert_at;
	size_t _insert_nm = insert_nm;
	size_t _nm_segments = nm_segments;
	header _left = *left();
	header _right = *right();

	TestRangeInsertedCorrectly(
		seg::insert_balance_left(left(), right(), m, s, insert_at, insert_nm),
		insert_nm,
		++left(),
		right());
}

TEST_F(TestSegmentInsert, InsertEmpty)
{
	size_t insert_nm = rand(1, 1000);
	auto r = seg::insert_empty(in, insert_nm);
	TestRangeInsertedCorrectly(
		r,
		insert_nm,
		left(),
		++right());
}

TEST_F(TestSegmentInsert, Insert)
{
	size_t segment_insert_nm = rand(0, 10);
	size_t insert_at_segment = 0;
	if (segment_insert_nm > 0) {
		InitRandRange(segment_insert_nm);
		size_t insert_at_segment = rand(0, segment_insert_nm - 1);
	}
	header_iterator insert_it = flat::successor(in.begin(), insert_at_segment);
	size_t insert_at_index = rand(seg::size(*insert_it));
	size_t insert_nm = rand(0, 1000);

	TestRangeInsertedCorrectly(
		seg::insert_to_segment_range(in, insert_it, insert_at_index, insert_nm),
		insert_nm);
}

#endif // INTERNAL_SEGMENTED_INSERT_TEST



#ifdef INTERNAL_SEGMENTED_ERASE_TEST

struct TestSegmentErase : public TestSegmentRangeBase
{
	void TestRangeErasedCorrectly(segmented_coordinate erase_pos, size_t left_size, size_t right_size)
	{
		first_inserted = begin();
		last_inserted = first_inserted;
		test_correct_segment_and_range_size(in.begin(), in.end());

		ASSERT_GE(erase_pos, begin()) <<
			"Erase pos is before the beginning of the range";

		ASSERT_LE(erase_pos, end()) <<
			"Erase pos is after the ending of the range";

		ASSERT_TRUE(seg::equal_flat_n_seg(v.begin(), left_size, begin(), erase_pos)) <<
			"Left range is not the same as before";

		ASSERT_TRUE(seg::equal_flat_n_seg(v.end() - right_size, right_size, erase_pos, end())) <<
			"Right range is not the same as before";
	}

	void TestRangeErasedCorrectly(
		std::pair<header_iterator, size_t> erase_pos,
		size_t left_size,
		size_t right_size)
	{
		segment_iterator it(erase_pos.first);
		return TestRangeErasedCorrectly(segmented_coordinate(it, flat::successor(std::begin(it), erase_pos.second)), left_size, right_size);
	}
};


TEST_F(TestSegmentErase, EraseCurrent)
{
	auto [first, last] = rand_begin_and_end(capacity);
	size_t size = last - first;
	size_t erase_nm = rand(size);
	size_t erase_at = rand(size - erase_nm);
	InitLeft(first, last);
	seg::erase_current(*left(), erase_at, erase_nm);
	TestRangeErasedCorrectly(seg::erase_balance_current(in, left(), erase_at), erase_at, size - (erase_at + erase_nm));
}

TEST_F(TestSegmentErase, Erase)
{
	size_t size = InitRandRange(rand(10));
	auto [firste, laste] = rand_begin_and_end(size);
	size_t erase_nm = laste - firste;
	segmented_coordinate first_erase = seg::successor(begin(), firste);
	segmented_coordinate last_erase = seg::successor(first_erase, erase_nm);

	header_iterator _first_erase = seg::segment(first_erase).h;
	size_t _first_i = seg::flat(first_erase) - seg::begin(*_first_erase);
	header_iterator _last_erase;
	size_t _last_i;

	if (last_erase == end() && begin() != end()) {
		_last_erase = --seg::segment(last_erase).h;
		_last_i = seg::size(*_last_erase);
	}
	else {
		_last_erase = seg::segment(last_erase).h;
		_last_i = seg::flat(last_erase) - seg::begin(*_last_erase);
	}
	auto [_it, _i, _s] = seg::erase_from_segment_range(in, _first_erase, _first_i, _last_erase, _last_i);
	TestRangeErasedCorrectly(std::make_pair(_it, _i), firste, size - laste);
}


#endif // INTERNAL_SEGMENTED_ERASE_TEST


#ifdef INTERNAL_SEARCH_TEST

struct TestSegmentContainerSearch : public InternalTestBase
{
	static multiset set;

	void SetUpSeg() override {
	}

	void TearDownSeg() override {
		set.clear();
	}

	void InitRand(size_t nm_segments) {
		index in;
		in.insert(in.begin(), static_cast<SizeType<index>>(nm_segments));
		size_t s = create_rand_range(in.begin(), nm_segments);
		set.swap(in, s);
	}
};

multiset TestSegmentContainerSearch::set;

TEST_F(TestSegmentContainerSearch, LowerBound)
{
	InitRand(rand(10));
	int size = static_cast<int>(set.size());
	segmented_coordinate end = set.end();
	for (int i = 0; i < size; ++i) {
		segmented_coordinate c = set.lower_bound(value_type(i));
		ASSERT_NE(c, end) << 
			"LowerBound was not found";
		ASSERT_GE(*c, value_type(i)) <<
			"Lesser element than LowerBound was found";
		ASSERT_EQ(*c, value_type(i)) <<
			"Greater element than LowerBound was found";
	}
}

TEST_F(TestSegmentContainerSearch, UpperBound)
{
	InitRand(10);
	int size = static_cast<int>(set.size());
	segmented_coordinate end = set.end();
	for (int i = 0; i < size; ++i) {
		segmented_coordinate c = set.upper_bound(value_type(i));
		if (i != size - 1) {
			ASSERT_NE(c, end) <<
				"UpperBound was not found";
			ASSERT_GE(*c, value_type(i + 1)) <<
				"Lesser element than UpperBound was found";
			ASSERT_EQ(*c, value_type(i + 1)) <<
				"Greater element than UpperBound was found";
		}
		else {
			ASSERT_EQ(c, end) <<
				"UpperBound was not found";
		}
	}
}

TEST_F(TestSegmentContainerSearch, EqualRange)
{
	InitRand(10);
	int size = static_cast<int>(set.size());
	segmented_coordinate end = set.end();
	for (int i = 0; i < size; ++i) {
		auto [lb, ub] = set.equal_range(value_type(i));
		ASSERT_NE(lb, end) <<
			"LowerBound was not found";
		ASSERT_GE(*lb, value_type(i)) <<
			"Lesser element than LowerBound was found";
		ASSERT_EQ(*lb, value_type(i)) <<
			"Greater element than LowerBound was found";

		if (i != size - 1) {
			ASSERT_GE(*ub, value_type(i + 1)) <<
				"Lesser element than UpperBound was found";
			ASSERT_EQ(*ub, value_type(i + 1)) <<
				"Greater element than UpperBound was found";
		}
		else {
			ASSERT_EQ(ub, end) <<
				"UpperBound was not found";
		}
	}
}

#endif

#ifdef EXTERNAL_COMPLETE_TEST


struct equal_to
{
	value_type x;
	equal_to(value_type x) : x(x) {}

	bool operator()(const value_type& y) const { return y == x; }
};

void check_equal_range(
	value_type x,
	const_segmented_coordinate seg_first, 
	const_segmented_coordinate seg_lb,
	const_segmented_coordinate seg_up,
	const_iterator first, 
	const_iterator lb,
	const_iterator ub)
{
	const_segmented_coordinate _seg_lb = seg::find_if_not(seg_lb, seg_up, equal_to(x));
	ASSERT_EQ(_seg_lb, seg_up) <<
		"Not all elements in the range [lower bound, upper bound) are equivalent";

	ASSERT_EQ(seg::distance(seg_first, seg_lb), lb - first) <<
		"Equivalent is not in the right positions";

	check_equal(
		seg::distance(seg_lb, seg_up),
		ub - lb,
		"Equivalent range is smaller than it should be",
		"Equivalent range is larger than it should be");
}

void non_decreasing_range(iterator first, iterator last, int lower_bound, int upper_bound)
{
	if (first == last || upper_bound == lower_bound)
		return;

	int value_range = upper_bound - lower_bound;
	int range_size = static_cast<int>(last - first);
	if (range_size > value_range) {
		int section_size = range_size / value_range;
		int last_section_size = section_size + range_size % value_range;
		int i = 0;
		while (i < value_range - 1) {
			first = std::fill_n(first, section_size, value_type(lower_bound + i));
			++i;
		}
		std::fill_n(first, last_section_size, value_type(lower_bound + i));
	}
	else {
		int step_size = value_range / range_size;
		int value = lower_bound;
		for (int i = 0; i < range_size; ++i) {
			*first = value_type(value);
			value += step_size;
			++first;
		}
	}
}

struct greater_than
{
	value_type x;
	greater_than(const value_type& x) : x(x) {}

	bool operator()(const value_type& y) { return y > x; }
};

static constexpr size_t max_value = 1000000;

struct TestSegmentContainerUsage : public InternalTestBase
{
	static multiset set;
	static std::vector<value_type> v;

	void CheckEqualContainers() {
		check_equal(
			set.size(),
			v.size(),
			"Segmented set is smaller than it should be",
			"Segmented set is larger than it should be");

		auto low_bound = std::begin(v);

		while (low_bound != std::end(v)) {
			auto up_bound = std::upper_bound(low_bound, std::end(v), *low_bound);
			auto x = set.equal_range(*low_bound);
			check_equal_range(*low_bound, std::begin(set), x.first, x.second, std::begin(v), low_bound, up_bound);
			low_bound = up_bound;
		}
	}

	void TearDownSeg() override {
		v.clear();
		set.clear();
	}

	void InitRand(size_t min_size, size_t max_size, int min_value, int max_value) {
		size_t size = rand(min_size, max_size);
		v.resize(size);
		non_decreasing_range(v.begin(), v.end(), min_value, max_value);
		set.insert_sorted_unguarded(set.begin(), v.begin(), v.size());
	}

	void EraseRand() {
		size_t erase_nm = rand(set.size());
		size_t erase_index = rand(0, set.size() - erase_nm);
		segmented_coordinate first = seg::successor(set.begin(), erase_index);
		set.erase(first, seg::successor(first, erase_nm));
		v.erase(flat::successor(v.begin(), erase_index), flat::successor(v.begin(), erase_index + erase_nm));
	}

	void InsertRand() {
		static constexpr size_t max_insert_nm = 500;

		if (!set.empty()) {
			size_t insert_nm = rand(max_insert_nm);
			size_t insert_index = rand(set.size());
			auto it = flat::successor(v.begin(), insert_index);
			int max = max_value;
			if (it != v.end()) {
				it = std::lower_bound(v.begin(), v.end(), *it);
				max = value_of(*it);
			}
			int min = it == v.begin() ? 0 : value_of(*(it - 1));
			it = v.insert(it, insert_nm, value_type(0));
			non_decreasing_range(it, flat::successor(it, insert_nm), min, max);
			set.insert_sorted_unguarded(seg::successor(set.begin(), it - v.begin()), it, insert_nm);
		}
		else {
			InitRand(1000, 2000, 0, static_cast<int>(rand(0, max_value)));
		}
	}

};

multiset TestSegmentContainerUsage::set;
std::vector<value_type> TestSegmentContainerUsage::v;

TEST_F(TestSegmentContainerUsage, Usage)
{
	InitRand(1000, 2000, 0, static_cast<int>(rand(0, max_value)));

	for (int i = 0; i < 100; ++i) {
		size_t r = rand(1);
		if (r == 0) {
			InsertRand();
		}
		else {
			EraseRand();
		}
		CheckEqualContainers();
	}

}

#endif

} // namespace test

} // namespace seg

} // namespace str2d


#include <functional>

int main(int argc, char** argv) {
	std::boyer_moore_searcher<char*> b(*argv, *argv);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#endif // SEG_TEST
