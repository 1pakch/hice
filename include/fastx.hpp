#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cstdio>

#include "hice/gzfile.hpp"


namespace hc {

struct fasta {
	std::string id;
	std::string nucleotides;
};

class buffered_reader {
    private:
	std::string buf_;
	gzfile f_;
	const char* line_start_;
	const char* buf_end_;

	void get_next_chunk() {
		auto raw_ptr = static_cast<void *>(const_cast<char *>(buf_.data()));
		auto bytes_read = f_.read(raw_ptr, buf_.capacity());
		buf_end_ = buf_.data() + bytes_read;
		line_start_ = buf_.data();
	}

	inline const char* next_newline() {
		auto p = line_start_;
		while ((p < buf_end_) && *p != '\n') ++p;
		return p;
	}

    public:

	buffered_reader(const char* fname, size_t bufsize)
		: f_(fname, "r", bufsize)
	{
		buf_.reserve(bufsize);
		get_next_chunk();
	}
	
	bool eof() const { return f_.eof() && line_start_ >= buf_end_ ; }
	
	std::string getline(size_t reserve_size) {
		std::string out;
		out.reserve(reserve_size);
		return getline(std::move(out));
	}

	std::string getline(std::string&& out=std::string()) {
		auto p = next_newline();
		out.append(line_start_, p - line_start_);
		if (*p == '\n' || eof()) {
			// newline found or reached EOF
			line_start_ = p + 1;
			return std::move(out);
		} else {
			// end of buffer but not the file
			get_next_chunk();
			return std::move(getline(std::move(out)));
		}
	}
	
	void skipline() {
		auto p = next_newline();
		if (*p == '\n' || eof()) {
			// newline found or reached EOF
			line_start_ = p + 1;
			return;
		} else {
			// end of buffer but not the file
			get_next_chunk();
			return skipline();
		}
	}

};


struct StoreSequences {
    std::vector<std::string> sequences;
    void operator()(size_t i, std::string&& id, std::string&& seq) {
        sequences.push_back(std::move(seq));
    }
};


class FastaParser {
    private:
	std::string buf_;
	gzfile f_;
        size_t n_ = 0;

	size_t get_next_chunk() {
		auto raw_ptr = static_cast<void *>(const_cast<char *>(buf_.data()));
		return f_.read(raw_ptr, buf_.capacity());
	}

    public:

	FastaParser(const char* fname, size_t bufsize)
		: f_(fname, "r", bufsize)
	{
		buf_.reserve(bufsize);
	}

        template<class ProcessRecord>
        void parse(ProcessRecord& process_record) {
            size_t chunk_size;
            bool at_description = true;
            std::string id; 
            std::string seq;
            while ((chunk_size = get_next_chunk())) {
                auto cur = &buf_[0];
                auto end = cur + chunk_size;
                while (cur < end) {
                    if (at_description) {
                        while (*cur != '\n' && cur < end) {
                            id.push_back(*cur++);
                        }
                        if (*cur == '\n') {
                            at_description = false;
                            ++cur;
                        }
                    } else {
                        while (*cur != '\n' && *cur != '>' && cur < end)
                            seq.push_back(*cur++);
                        if (*cur == '>') {
                            process_record(n_, std::move(id), std::move(seq));
                            id = std::string();
                            seq = std::string();
                            at_description = true;
                            ++n_;
                            ++cur;
                        } else if (*cur == '\n') {
                            ++cur;
                        }
                    }
                }
                if (chunk_size < buf_.capacity()) {
                    if (!at_description && seq.size())
                        process_record(n_, std::move(id), std::move(seq));
                    break;
                }
            }
        }
};

}
