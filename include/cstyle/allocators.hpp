#pragma once

#include <cassert>
#include <cstdlib>

#include <hice/traits.hpp>


namespace genlib {

//! TODO: add assertions for failed allocations
struct mallocator {

	static void* malloc(size_t n) { return std::malloc(n); }

	static void* realloc(void *p, size_t n){ return std::realloc(p, n); }

	static void free(void *p){ std::free(p); }
};

struct noalloc {
	static void free(void *p) {}
};



template<typename T, typename Allocator>
T* typed_malloc(size_t n_items) {
	auto n_bytes = n_items * sizeof(T); 
	return (T*) Allocator::malloc(n_bytes);
}

template<typename T, typename Allocator>
T* typed_realloc(T* p, size_t n_items) {
	auto n_bytes = n_items * sizeof(T); 
	return (T*) Allocator::realloc((void*) p, n_bytes);
}

}
