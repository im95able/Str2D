#pragma once
#include <algorithm>


struct instrumented_base
{
	enum operations {
		conversion,
		default_constructor,
		copy_constructor,
		move_constructor,
		constructor,
		assignment,
		destructor,
		equality,
		comparison,
		exist,
		number_ops
	};
	static size_t counts[number_ops];
	static size_t snapshot[number_ops];
	static const char* counter_names[number_ops];
	static void init() {
		std::fill(std::begin(counts), std::end(counts), 0);
		std::fill(std::begin(snapshot), std::end(snapshot), 0);
	}
	static void save_snapshot() {
		std::copy(std::begin(counts), std::end(counts), std::begin(snapshot));
	}
};

template <typename T>
// T is Semiregualr or Regular or TotallyOrdered
struct instrumented : instrumented_base
{
	typedef T value_type;
	T value;

	// Conversion:
	explicit instrumented(const value_type& v) : value(v) {
		++counts[constructor];
		++counts[conversion]; 
		++counts[exist];
	}

	// Semiregular:
	instrumented() {
		++counts[constructor];
		++counts[default_constructor];
		++counts[exist];
	}
	instrumented(const instrumented& x) : value(x.value) {
		++counts[constructor];
		++counts[copy_constructor];
		++counts[exist];
	}
	instrumented(instrumented&& x) noexcept : value(std::move(x.value)) {
		++counts[constructor];
		++counts[move_constructor];
		++counts[exist];
	}
	~instrumented() { 
		++counts[destructor];
		--counts[exist];
	}
	instrumented& operator=(const instrumented& x) {
		++counts[assignment];
		value = x.value;
		return *this;
	}
	// Regular
	friend
	bool operator==(const instrumented& x, const instrumented& y) {
		++counts[equality];
		return x.value == y.value;
	}
	friend
	bool operator!=(const instrumented& x, const instrumented& y) {
		return !(x == y);
	}
	// TotallyOrdered
	friend
	bool operator<(const instrumented & x, const instrumented & y) {
		++counts[comparison];
		return x.value < y.value;
	}
	friend
	bool operator>(const instrumented & x, const instrumented & y) {
		return y < x;
	}
	friend
	bool operator<=(const instrumented & x, const instrumented & y) {
		return !(y < x);
	}
	friend
	bool operator>=(const instrumented & x, const instrumented & y) {
		return !(x < y);
	}
};

size_t instrumented_base::counts[number_ops];
size_t instrumented_base::snapshot[number_ops];
const char* instrumented_base::counter_names[number_ops] = {
	"conversion",
	"default_constructor",
	"copy_constructor",
	"move_constructor",
	"constructor"
	"assignment",
	"destructor",
	"equality",
	"comparison",
	"exist"
};