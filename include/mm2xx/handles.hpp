// handles.cpp - smart-pointers wrapping minimap2 pointer types

#pragma once
#include <memory>
#include <minimap.h>


namespace mm {

// Define deleter structs for unique_ptr's wrapping mimimap2 pointer types
namespace impl {

// Allocations handled by C++ should use new and delete
template<typename T>
struct deleter: public std::default_delete<T> {};

// Pointers returned by minimap2 should use be freed accordingly
template<> struct deleter<mm_idx_t> {
	void operator()(mm_idx_t *p){ mm_idx_destroy(p); }
};

template<> struct deleter<mm_tbuf_t> {
	void operator()(mm_tbuf_t *p){ mm_tbuf_destroy(p); }
};

template<> struct deleter<mm_reg1_t> {
	void operator()(mm_reg1_t *p) { std::free(p); }
};

template<typename T> struct item_destructor {
	void operator()(T* p) {}
};

template<> struct item_destructor<mm_reg1_t> {
	void operator()(mm_reg1_t* p) { std::free(p->p); }
};

} // impl


//! unique_ptr wrapping minimap2 pointer types with approriate deleters
template<typename T>
class handle: public std::unique_ptr<T, impl::deleter<T>> {
    public:
	using std::unique_ptr<T, impl::deleter<T>>::unique_ptr;
};

//! unique_ptr wrapping C array with a possibility to free each item
template<typename T>
class handle<T[]>: public std::unique_ptr<T[], impl::deleter<T>> {
    private:
	using Base = std::unique_ptr<T[], impl::deleter<T>>;
	size_t size_; // to be ignored if base pointer is null

    public:
	handle() {} // base pointer is null-initialized
	
	handle(T *p, size_t size): Base(p), size_(size) {}
	
	handle(handle&& v): Base(std::move(v)), size_(v.size_) {}

	handle& operator=(handle&& v) {
		Base::operator=(std::move(v));
		size_ = v.size_;
		return *this;
	}

	~handle() {
		impl::item_destructor<T> delitem;
		for (auto p=begin(); p < end(); ++p)
			delitem(p);
	}

	size_t empty() const { return !this->get(); }
	size_t size() const { return empty() ? 0 : size_; }

	T* begin() { return this->get(); }
	T* end() { return this->get() + size(); }
	const T* cbegin() const { return begin(); }
	const T* cend() const { return end(); }
};

} // mm
