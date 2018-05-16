#pragma once

#include <string>
#include <cmath>

#include <zlib.h>

#include <hice/exception.hpp>


namespace hc {

class GzError: public hc::Exception { using hc::Exception::Exception; };

class gzfile {
    private:
	const std::string path_;
	const std::string mode_;
	gzFile f_;

    public:
	gzfile(const char *path, const char *mode, unsigned bufsize)
		: path_(path)
		, mode_(mode)
		, f_(gzopen(path, mode))
	{
		if (!f_) throw GzError(__PRETTY_FUNCTION__, "gzopen() failed",
				       path, mode_);
		if (Z_OK != gzbuffer(f_, bufsize))
			throw GzError(path_, "gzbuffer() failed", bufsize);
	}

	size_t read(void *buf, unsigned len){
		auto result = gzread(f_, (void *)buf, len);
		if (result < Z_OK) {
			throw GzError(path_, "gzread() failed", buf, len);
		}
		return std::abs(result);
	};
	
	~gzfile(){}

	const char* path() const { return path_.c_str(); }
	const char* mode() const { return mode_.c_str(); }

	bool eof() const { return gzeof(f_); };
	int offset() const { return gzoffset(f_); };
};

}
