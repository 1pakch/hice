#pragma once

#include <memory>

namespace hc {

namespace impl {

//! Defines a deleter struct compaitble with std::unique_ptr
template<typename T, void (*f)(T* p)>
struct custom_deleter {
	inline void operator()(T* p){ f(p); }
};

}

// Untyped producers/consumers wrapped via std::unique_ptr
template<typename T, void (*f)(T* p)>
using unique_ptr = std::unique_ptr<T, impl::custom_deleter<T, f>>;

}
