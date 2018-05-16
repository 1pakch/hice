#pragma once

#include <ctime>
#include <sstream>
#include <ostream>

// captures function name via macro
#define TIMEIT(f, ...) timeit(#f, f, __VA_ARGS__)


namespace hc {

namespace fmt {

void append_sep(std::ostream& o, const char *sep) {}

template<typename T>
void append_sep(std::ostream& o, const char *sep, const T& t) {
	o << sep << t; 
}

template<typename T, typename... Args>
void append_sep(std::ostream& o, const char *sep, const T& t, const Args&... args) {
	append_sep(o, sep, t);
	append_sep(o, sep, args...);
}

template<typename T, typename... Args>
void join(std::ostream& o, const char *sep, const T& t, const Args&... args) {
	o << t;
	append_sep(o, sep, args...);
}

void join(std::ostream& o, const char *sep) {}

}

template<typename T>
class timed {
    private:
	T result_;
	std::string callstr_;
	double seconds_;
    public:
	timed(T&& result, std::string&& callstr, clock_t start)
		: result_(std::move(result))
		, callstr_(std::move(callstr))
		, seconds_(double(clock()-start)/CLOCKS_PER_SEC)
	{}
	const timed& print() const {
		std::printf("timeit: %s took %.2f seconds\n", callstr_.c_str(), seconds_);
		return *this;
	}
	const T& result() const { return result_; }
};

template<typename Result, typename... Args>
timed<Result> timeit(const char* fname, Result (*f)(Args...), Args... args)
{
	std::stringstream s;
	s << fname << '(';
	fmt::join(s, ", ", args...);
	s << ')';

	clock_t start = clock();
	Result result = f(std::forward<Args>(args)...);
	return timed<Result>(std::move(result), std::move(s.str()), start);
}


}
