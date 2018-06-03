// handles.cpp - smart-pointers wrapping minimap2 pointer types

#pragma once

#include <memory>
#include <minimap.h>
#include <cstdio>


namespace mm {

namespace impl {

// Require explicit deallocator specialization for all wrapped types
// (by not providing the operator()(T* p) by default).
template<typename T> struct deallocate {};

// These pointers are returned by mm2 and should be freed accordingly
template<> struct deallocate<mm_idx_t> {
    void operator()(mm_idx_t *p) { mm_idx_destroy(p); }
};

template<> struct deallocate<mm_tbuf_t> {
    void operator()(mm_tbuf_t *p) { mm_tbuf_destroy(p); }
};

template<> struct deallocate<mm_reg1_t> {
    void operator()(mm_reg1_t *p) {
        std::free(p);
    }
};

// Option structs will be allocated by us. Hence delegate to deleter.
template<>
struct deallocate<mm_idxopt_t> : public std::default_delete<mm_idxopt_t> {};

template<>
struct deallocate<mm_mapopt_t> : public std::default_delete<mm_mapopt_t> {};


//! Default destructor is noop
template<typename T> struct destruct {
    void operator()(T *p) {}
};

//! mm_reg1_t contains a pointer to mm_extra_t that should be freed.
template<> struct destruct<mm_reg1_t> {
    void operator()(mm_reg1_t *p) {
        std::free(p->p);
    }
};

} // namespace impl


//! unique_ptr that calls externally specified destructor
template<typename T>
class handle : public std::unique_ptr<T, impl::deallocate<T>> {
  private:
    using Base = std::unique_ptr<T, impl::deallocate<T>>;

    void _destruct_contents() {
        impl::destruct<T> destruct;
        destruct(this->get());
    }

  public:
    using std::unique_ptr<T, impl::deallocate<T>>::unique_ptr;

    handle(handle &&v) noexcept
        : Base(std::move(v)) {}

    handle &operator=(handle &&v) noexcept {
        _destruct_contents();
        Base::operator=(std::move(v));
        return *this;
    }

    handle(const handle&) = delete;
    handle &operator=(const handle&) = delete;

    ~handle() { _destruct_contents(); }
};

//! unique_ptr for an array of pointers possibly destructing items
template<typename T>
class handle<T[]> : public std::unique_ptr<T[], impl::deallocate<T>> {
  private:
    using Base = std::unique_ptr<T[], impl::deallocate<T>>;
    size_t size_ = 0; // to be ignored if base pointer is null

    void _destruct_contents() {
        impl::destruct<T> destruct;
        for (auto p = begin(); p < end(); ++p)
            destruct(p);
    }

  public:
    handle() {} // base pointer is null-initialized

    handle(T *p, size_t size) noexcept
        : Base(p)
        , size_(size) {}

    handle(handle &&v) noexcept
        : Base(std::move(v))
        , size_(v.size_) {}

    handle &operator=(handle &&v) noexcept {
        _destruct_contents();
        Base::operator=(std::move(v));
        size_ = v.size_;
        return *this;
    }

    handle(const handle&) = delete;
    handle &operator=(const handle&) = delete;

    ~handle() { _destruct_contents(); } 

    size_t empty() const { return !this->get(); }
    size_t size() const { return empty() ? 0 : size_; }

    T *begin() { return this->get(); }
    T *end() { return this->get() + size(); }
    const T *cbegin() const { return begin(); }
    const T *cend() const { return end(); }
};

} // namespace mm
