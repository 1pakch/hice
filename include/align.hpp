#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>

#include <minimap2/minimap.h>
#include <mm2xx/mapped.hpp>
#include <mm2xx/encoding.hpp>
#include <mm2xx/cswriter.hpp>


extern uint8_t seq_nt4_table[256]; // encoding from 'ACGT' to internal

namespace mm {

const std::string empty_string_;

#pragma pack(push,1)
template<class Mapped=Mapped<>>
class Aligned : public Mapped {
    using query_size_type = typename Mapped::query_size_type;
    query_size_type qlen_;  // Length of query string
    query_size_type ldist_; // Levenshtein distance
    std::unique_ptr<std::string> cs_;
    std::unique_ptr<std::string> qs_str_; // query[:qs_] where query is possibly rc-d
    std::unique_ptr<std::string> qe_str_; // query[(qs_+qlen_):] where query is possibly rc-d
  public:
    Aligned()
        : Mapped() {}

    Aligned(const mm_reg1_t *reg, query_size_type qlen)
        : Mapped(reg)
        , qlen_(qlen)
    {}

    Aligned(Aligned&&) = default;
    Aligned& operator=(Aligned&&) = default;
    Aligned(const Aligned&) = delete;
    Aligned& operator=(const Aligned&) = delete;

    size_t ldist() const { return ldist_; }

    size_t qstart() const { return qs_str_? qs_str_->size() : 0; }
    size_t qend() const { return qlen() - (qe_str_? qe_str_->size() : 0); }
    size_t qlen() const { return qlen_; }

    const std::string& cs() const { return cs_? *(cs_.get()) : empty_string_; }
    const std::string& qs_str() const { return qs_str_? *(qs_str_.get()) : empty_string_; }
    const std::string& qe_str() const { return qe_str_? *(qe_str_.get()) : empty_string_; }

    void set_cs(std::string&& cs) {
        cs_ = std::make_unique<std::string>(std::move(cs));
    }

    void set_qs(std::string&& qs) {
        qs_str_ = std::make_unique<std::string>(std::move(qs));
    }

    void set_qe(std::string&& qe) {
        qe_str_ = std::make_unique<std::string>(std::move(qe));
    }

    void set_ldist(size_t ldist) {
        ldist_ = ldist;
    }

};
#pragma pack(pop)


class AlignerBase: public MapperBase {

    protected:
	std::basic_string<enc::encoded> ref_buf_;
	std::basic_string<enc::encoded> qbuf_;

	using MapperBase::MapperBase;

	// Encode the whole query into the query buffer 
	void load_ref(uint8_t rid, uint32_t start, uint32_t end) {
		ref_buf_.reserve(end - start);
		mm_idx_getseq(idx(), rid, start, end, &ref_buf_[0]);
	}

};


/// Aligner returning the best alignment for a given query
class Aligner: public AlignerBase {
  public:
    Aligner(const Settings& settings): AlignerBase(settings, 1) {}  
    Aligned<> map(std::string const& query) {
        auto hits = MapperBase::map(query);
        if (hits.size()) {
            auto hit = hits[0];
            auto qlen = query.size();
            Aligned<> result(&hit, qlen); 
            enc::encode_str(qbuf_, query, hit.rev);
            load_ref(hit.rid, hit.rs, hit.re);
            size_t offset = hit.rev ? query.size() - hit.qe : hit.qs;
            //printf("rev=%d, qs=%d, qe=%d, qlen=%zd\n", hit.rev, hit.qs, hit.qe, qlen);
            //fflush(stdout);
            CigarIterator ci(&ref_buf_[0], &qbuf_[offset], hit.p);
            CsStringWriter csw;
            ci.map(csw);
            result.set_ldist(ci.ldist());
            if (ci.ldist())
                result.set_cs(std::move(csw.result()));
            if (hit.qs != 0)
                result.set_qs(std::move(enc::decode_str(qbuf_, 0, hit.qs)));
            if (hit.qe != qlen)
                result.set_qe(std::move(enc::decode_str(qbuf_, hit.qe, qlen)));
            return std::move(result);
        } else {
            return Aligned<>();
        }
    }
};


} // namespace mm
