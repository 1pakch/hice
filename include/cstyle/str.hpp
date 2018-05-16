#pragma once

#include <cstring>
#include <cstdarg>

#include <hice/allocators.hpp>
#include <hice/buffer.hpp>
#include <hice/posix.hpp>


namespace hc {

using str = genlib::buf<char, genlib::mallocator>;
using strview = genlib::view<char>;

namespace impl {

//! Rounds up to the next power of 2.
size_t upper_exp2(size_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
};

}

struct growingstr {

    private:

	str buf_;
	size_t length_ = 0;

    public:

	growingstr(): buf_() {}
	growingstr(size_t size): buf_(size) {}
	growingstr(str&& s): buf_(s.move()), length_(s.size()) {}

	char* ptr() const { return buf_.ptr(); }

	size_t length() const { return length_; }
	size_t capacity() const { return buf_.size(); }
	size_t space_left() const { return (capacity() - length()); }

	growingstr(growingstr&& other)
		: buf_(other.buf_.move())
		, length_(other.length_)
	{}

	growingstr&& move() { return static_cast<growingstr&&>(*this); }

	str&& freeze() {
		if (length_ && capacity() > length())
			buf_.realloc(length_);
		length_ = 0;
		return buf_.move();
	}

	~growingstr() {
		//std::printf("~:%s(%d)\n", ptr(), length());
	};
	
	//! Ensure that at least this bytes are available
	void ensure_space(size_t s) { if (space_left() < s) expand(s); }
	
	//! Expand the buffer by at least this amount
	void expand(size_t by) {
		size_t new_cap = impl::upper_exp2(capacity() + by);
		if (new_cap < 128) new_cap = 128;
		buf_.realloc(new_cap);
	}

	growingstr&& append(char c) {
		ensure_space(1);
		buf_.ptr()[length_++] = c;
		return this->move();
	}

	growingstr&& append(const char* src, size_t s) {
		ensure_space(s);
		memcpy(buf_.ptr() + length(), src, s);
		length_ += s;
		return this->move();
	}
	
	void append(const char* chars) {
		while (*chars != '\0')
			append(*chars++);
	}
	
	template<typename S>
	void append(const S& s) {
		ensure_space(s.size());
		append(s.ptr(), s.size());
	}

	int printf(bool keep_null, const char* fmt, ...) {
		std::va_list args, backup;
		va_start (args, fmt);
		va_copy (backup, args);
		auto left = int(space_left());
		// need to have one more byte because of 0 terminator
		auto needed = 1 + vsnprintf(ptr()+length(), left, fmt, args);
		if (needed < 0) return needed; // error
		else if (needed > left) {
			expand(needed);
			vsnprintf(ptr()+length(), needed, fmt, backup);
		}
		if (!keep_null) needed -= 1;
		length_ += needed;
		va_end(args);
		va_end(backup);
		return needed;
	}
	
	void fdwrite(int fd=0, bool flush=false) {
		write(fd, ptr(), length());
		if (flush) fdatasync(fd);
	}

};


}
