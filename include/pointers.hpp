#pragma once

#include <cassert>
#include <hice/traits.hpp>
#include <hice/allocators.hpp>


namespace genlib {

template<typename PT, typename Allocator>
class owned {
    private:
	PT p_;
	
	static bool fail_for_non_pointer_types() { return is_pointer<PT>::value; }

    public:

	owned(): p_(nullptr) {}

	//! Takes ownership of the argument pointer.
	//! It is assumed that p can be freed using allocator A.
	owned(PT p): p_(p) {}

	//! Takes ownership from the other
	owned(owned&& other): p_(other.p_) { other.p_ = nullptr; }

	//! Swaps owned pointers between this and the other
	owned& operator=(owned&& other){
		other.p_ = replace_ptr(other.p_);
		return *this;
	}
	
	//! Frees the memory. XXX: we don't know the size to call destructors
	~owned() {
		//using T = typename remove_pointer<PT>::type;
		//p_->~T();
		Allocator::free((void *)p_);
	}
	
	/// Disable copy constructors and assignments
	owned(const owned&) = delete;
	owned& operator=(const owned&) = delete;

	bool null() const { return !p_; }
	operator bool() const { return p_; }

	//! Returns a raw pointer without transferring ownership
	PT ptr() const { return p_; }
	
	//! Returns a raw pointer and stops managing it
	PT release_ptr() { return replace_ptr(nullptr); }

	//! Returns a raw pointer and starts managing the argument instead
	PT replace_ptr(PT new_) {
		auto old = p_;
		p_ = new_;
		return old;
	}

	//! Allows the caller to move a bound value
	owned&& move() { return static_cast<owned&&>(*this); }

};


}

