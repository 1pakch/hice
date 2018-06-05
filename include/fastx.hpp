#pragma once

#include <string>
#include <cstring>
#include <utility>
#include <vector>
#include <cstdio>
#include <cassert>
#include <regex>

#include <bpcqueue.hpp>

#include "hice/gzfile.hpp"
#include "hice/tagged.hpp"


namespace hc {

namespace fastx {

enum class Format { fasta, fastq, ambiguous };

namespace impl {
    static const std::regex fasta_path(R"(.*\.(fa|fasta)(.gz)?$)");
    static const std::regex fastq_path(R"(.*\.(fq|fastq)(.gz)?$)");
}

Format guess_format(const char *path) {
    bool is_fasta = std::regex_match(path, impl::fasta_path);
    bool is_fastq = std::regex_match(path, impl::fastq_path);
    if (is_fasta != is_fastq) return is_fasta ? Format::fasta : Format::fastq;
    else return Format::ambiguous;
}

namespace impl {

template<Format format> struct FormatTraits {};

template<> struct FormatTraits<Format::fasta> { 
    enum class Field { id, sequence };
    static constexpr int n_fields = 2;

    static inline constexpr Field next_field(Field cur) {
        auto new_ = static_cast<Field>((static_cast<int>(cur) + 1) % n_fields);
        //printf("fasta::next_field(%d) = %d\n", static_cast<int>(cur), static_cast<int>(new_));
        return new_;
    }

    static inline void init_state(Field& f, size_t& skip, bool& field_over) {
        f = Field::id;
        skip = 1;
        field_over = false;
    }

    static inline void on_char(Field f, char c, size_t& skip, bool& field_over){
        skip = c == '\n' || c == '>';
        field_over = (f == Field::id && c == '\n') || (f == Field::sequence && c == '>' );
        //printf("on_char(%d, %c, %d, %d)\n", static_cast<int>(f), c, skip, field_over);
    }
};

template<> struct FormatTraits<Format::fastq> {
    
    enum class Field { id, sequence, thirdline, quality };
    static constexpr int n_fields = 4;
    
    static inline constexpr Field next_field(Field cur) {
        auto new_ = static_cast<Field>((static_cast<int>(cur) + 1) % n_fields);
        //printf("fastq::next_field(%d) = %d\n", static_cast<int>(cur), static_cast<int>(new_));
        return new_;
    }

    static inline void init_state(Field& f, size_t& skip, bool& field_over) {
        f = Field::id;
        skip = 1;
        field_over = false;
    }

    static inline void on_char(Field f, char c, size_t& skip, bool& field_over){
        skip = (c == '\n');
        if (f == Field::sequence || f == Field::quality)
            skip *= 2; // skip @ and + in lines id and thirdline respectively
        field_over = skip;
    }
};

}

template<Format format>
class Parser {
    private:
	std::string buf_;
	gzfile f_;

	size_t get_next_chunk() {
		auto raw_ptr = static_cast<void *>(const_cast<char *>(buf_.data()));
		size_t n_bytes = f_.read(raw_ptr, buf_.capacity());
                return n_bytes;
	}

        using FT = impl::FormatTraits<format>;

    public:

	Parser(const char* fname, size_t bufsize)
		: f_(fname, "r", bufsize)
	{
		buf_.reserve(bufsize);
	}

        template<class Processor>
        void parse(Processor& processor) {
            size_t chunk_size;
            typename FT::Field field;
            size_t skip;
            bool field_over;
            FT::init_state(field, skip, field_over);
            do {
                chunk_size = get_next_chunk();
                auto cur = &buf_[0];
                auto end = cur + chunk_size;
                for ( ; cur < end; ++cur) {
                    if (skip) {
                        --skip;
                    } else {
                        FT::on_char(field, *cur, skip, field_over);
                        if (!skip) processor.on_char(field, *cur);
                        else --skip;
                        if (field_over) {
                            processor.on_field_over(field);
                            field = FT::next_field(field);
                        }
                    }
                } 
            } while (chunk_size == buf_.capacity());
            if (!field_over) processor.on_field_over(field);
            //processor.on_eof();
        }
};


template<class OnSequence, size_t reservation_size=0>
struct SequenceExtractor {
  private:
    std::string current_;
    OnSequence& callback_;

    void reset() {
        current_ = std::string();
        if (reservation_size) current_.reserve(reservation_size);
    }

  public:
    SequenceExtractor(OnSequence& callback) 
        : callback_(callback) 
    {
        reset();
    }

    template<class Field>
    inline void on_char(Field f, char c) {
        //std::printf("%d %c\n", f, c);
        //fflush(stdout);
        if (f != Field::sequence) return;
        current_.push_back(c);
    }

    template<class Field>
    inline void on_field_over(Field f) {
        if (f != Field::sequence) return;
        //printf("Extracted %s\n", current_.c_str());
        //fflush(stdout);
        callback_(std::move(current_));
        reset();
    }

    inline void on_eof() {
        if (current_.size())
            callback_(std::move(current_));
    }
};


struct StoreSequences {
    std::vector<std::string> sequences;
    void operator()(std::string&& s) { sequences.push_back(std::move(s)); }
};

class StreamSequences {
    bpcqueue::Output<std::string> output_;
  public:
    StreamSequences(bpcqueue::Output<std::string>&& output) : output_(std::move(output)) {}
    void operator()(std::string&& seq) {
        output_.push(std::move(seq));
    }
};

void parse(Tagged<const char*, Format> path,
           bpcqueue::Output<std::string>&& output,
           size_t bufsize) {
    StreamSequences streamer(std::move(output));
    SequenceExtractor<StreamSequences> extractor(streamer);
    switch (path.tag) {
        case Format::fasta : {
                Parser<Format::fasta> p(path.value, bufsize);
                p.parse(extractor);
                break;
            }
        case Format::fastq : {
                Parser<Format::fastq> p(path.value, bufsize);
                p.parse(extractor);
                break;
            }
        default:
            assert(0);
    }
}

}

}
