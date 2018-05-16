# pragma once

namespace genlib {

template<typename T>
struct is_pointer {};

template<typename T>
struct is_pointer<T*> { static const bool value = true; };

template<typename T>
struct remove_pointer { using type = T; };

template<typename T>
struct remove_pointer<T*> { using type = T; };

}
