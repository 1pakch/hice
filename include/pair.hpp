#pragma once

#include <utility>
#include <type_traits>

#include <bpcqueue.hpp>


namespace bpcqueue {

//! A pair that is noexcept move-constructible and move-assignable
template<typename T>
class Pair: private std::pair<T, T> {
    using Base = std::pair<T, T>;
  public:
    using Base::pair;
 
    Pair(Pair&& other) noexcept
        : Base::pair(std::move(other))
    {}
    
    Pair(T&& first, T&& second) noexcept
        : Base::pair(std::move(first), std::move(second))
    {}

    Pair& operator=(Pair&& other) noexcept {
        this->first = std::move(other.get_first());
        this->second = std::move(other.get_second());
        return *this;
    }
    
    T get_first() { return std::move(this->first); }
    T get_second() { return std::move(this->second); }
};


template<typename T, template<typename> class Pair=Pair>
void pair(Input<T> src1, Input<T> src2, Output<Pair<T>> dst) {
    T v1, v2;
    while (src1.pop(v1) && src2.pop(v2))
        dst.emplace(std::move(v1), std::move(v2));

}

template<typename T, template<typename> class Pair=Pair>
void unpair(Input<Pair<T>> src, Output<T> dst) {
    Pair<T> pair_;
    while (src.pop(pair_)) {
        dst.push(pair_.get_first());
        dst.push(pair_.get_second());
    }
}

}

