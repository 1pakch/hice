#pragma once

#include <cstdlib>

#include <hice/allocators.hpp>
#include <hice/pointers.hpp>


namespace genlib {

template<typename T, typename Allocator> class buf;

//! A non-owning view on existing buffer;
template<typename T>
class view {
    private:

	T* p_;
	size_t size_;

    public:

	view(T* p, size_t size): p_(p), size_(size) {}

	size_t size() const { return null() ? 0 : size_; }

	T* ptr() const { return p_; }

	bool null() const { return !p_; }

	operator bool() const { return p_; }
	
	view&& move() { return static_cast<view&&>(*this); }

	template<typename Allocator>
	auto copy() const { return buf<T, Allocator>::copy(*this); }
};

//! A buffer owning the memory behind.
template<typename T, typename Allocator>
class buf {
    private:
	owned<T*, Allocator> op_;
	size_t size_;
    public:

	buf() = default;

	buf(T* p, size_t size): op_(p), size_(size) {}

	buf(size_t size): op_(typed_malloc<T, Allocator>(size)), size_(size) {}

	buf(buf&& other) = default;

	buf& operator=(buf&& other) = default;

	size_t size() const { return null() ? 0 : size_; }

	T* ptr() const { return op_.ptr(); }
	T* release_ptr() { return op_.release_ptr(); }
	buf&& move() { return static_cast<buf&&>(*this); }

	void realloc(size_t size) {
		op_.replace_ptr(typed_realloc<T, Allocator>(op_.ptr(), size));
		size_ = size;
	}

	bool null() const { return op_.null(); }
	operator bool() const { return !null(); }

	genlib::view<T> view() const {
		return genlib::view<T>(ptr(), size());
	}

	genlib::view<T> view(size_t start, size_t length) {
		return genlib::view<T>(ptr() + start, length);
	}

	static buf&& copy(const char *s, size_t size) {
		if (!s || !size) return buf().move();
		auto result = buf(size);
		memcpy(result.ptr(), s, size*sizeof(T));
		return result.move();
	}

	template<typename S>
	buf&& copy(const S& s) { return copy(s.ptr(), s.size()).move(); }
};

}
