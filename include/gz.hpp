#pragma once

#include <string>
#include <cmath>
#include <type_traits>

#include <zlib.h>

#include <hice/exception.hpp>
#include <hice/pipe.hpp>
#include <hice/unique_ptr.hpp>


namespace hc {

class GzError: public hc::Exception {
	using hc::Exception::Exception;
};

void gzclose_void(gzFile f){ return; }

class gzfile {
    private:
	const std::string path_;
	const std::string mode_;
	hc::unique_ptr<gzFile_s, noop_deleter> f_;

    public:
	gzfile(const char *path, const char *mode, unsigned bufsize)
		: path_(path)
		, mode_(mode)
		, f_(gzopen(path, mode))
	{
		if (!f_) throw GzError(__PRETTY_FUNCTION__, "gzopen() failed",
				       path, mode_);
		if (Z_OK != gzbuffer(f_.get(), bufsize))
			throw GzError(path_, "gzbuffer() failed", bufsize);
	}

	gzfile(gzfile&&) = default;
	gzfile& operator=(gzfile&&) = default;

	~gzfile() { if (f_) close(); }

	size_t read(void *buf, unsigned len){
		auto result = gzread(f_.get(), (void *)buf, len);
		if (result < Z_OK) {
			throw GzError(path_, "gzread() failed", buf, len);
		}
		return std::abs(result);
	};
	
	std::string readstr(unsigned len, size_t *bytes_read){
		auto s = std::string();
		s.resize(len);
		*bytes_read = gzread(f_.get(), (void *)&s[0], len);
		if (*bytes_read < 0) {
			throw GzError(path_, "gzread() failed", &s[0], len);
		}
		s.resize(*bytes_read);
		return std::move(s);
	};

	void close() {
		auto p = f_.release();
		if (!p) return;
		int err = gzclose(p);
		if(Z_OK != err) throw GzError(path_, "gzclose() failed", 0);
	}

	const char* path() const { return path_.c_str(); }
	const char* mode() const { return mode_.c_str(); }

	bool eof() const { return gzeof(f_.get()); };
	int offset() const { return gzoffset(f_.get()); };
};


void gz_read_into(Producer<std::string> p, gzfile f, size_t chunk_size) {
	size_t bytes_read;
	do {
		p.push(std::move(f.readstr(chunk_size, &bytes_read)));
	} while (bytes_read == chunk_size);
}


}
