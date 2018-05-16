#pragma once

#include <stdexcept>
#include <sstream>
#include <string>

#include <cxxabi.h>

namespace hc {

namespace impl {

template<typename T>
std::string type_name() {
	int status;
	const char *mangled = typeid(T).name();
	char *demangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);
	auto result = std::string(0==status ? demangled : mangled);
	if (!status) std::free(demangled);
	return std::move(result);
}

template<typename T>
std::ostream& logargs(int i, std::ostream& o, const T& t) {
	o << "  arg" << i << " = ";
	o << "\"" << t << "\" ";
	o << "(" << type_name<T>() << ")\n";
	return o;
}

template<typename T, typename... Args>
std::ostream& logargs(int i, std::ostream& o, T t, const Args&... args) {
	logargs(i, o, t);
	logargs(i+1, o, args...);
	return o;
}

template<typename T1, typename T2, typename... Args>
std::string exception_message(const T1 where, const T2 reason,
				const Args&... args) {
	std::stringstream o;
	o << reason << " in/for " << where << '\n';
	logargs(0, o, args...);
	return std::move(o.str());
}

}

struct Exception: public std::exception {

	const std::string what_;

	template<typename T1, typename T2, typename... Args>
	Exception(const T1 where, const T2 reason, const Args&... args)
		: what_(impl::exception_message(where, reason, args...))
	{}

	const char* what() const noexcept { return what_.c_str(); }
};

} // namespace

