#pragma once
#include <random>
#include <chrono>

static std::default_random_engine rand_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

size_t rand(size_t min, size_t max) {
	return std::uniform_int_distribution<size_t>(min, max)(rand_engine);
}

size_t rand(size_t max) {
	return std::uniform_int_distribution<size_t>(0, max)(rand_engine);
}