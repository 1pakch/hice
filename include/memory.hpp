#pragma once

#include <memory>

namespace hc {

template<typename T>
struct no_deleter {
    void operator()(T* p){}
};

}
