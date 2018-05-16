#pragma once

#include <string>
#include <cxxabi.h>

namespace hc {

template<typename T>
std::string type_name() {
	int status;
	const char *mangled = typeid(T).name();
	char *demangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);
	auto result = std::string(0==status ? demangled : mangled);
	if (!status) std::free(demangled);
	return std::move(result);
}

}
