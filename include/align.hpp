#include <string>
#include <sstream>
#include <cstdint>
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
    query_size_type ldist_; // Levenshtein distance
    query_size_type qlen_;  // Length of query string
    std::unique_ptr<std::string> cs_;
    std::unique_ptr<std::string> qs_str_; // query_string[:qs_]
    std::unique_ptr<std::string> qe_str_; // query_string[(qs_+qlen_):]
  public:
    Aligned()
        : Mapped() {}

    Aligned(const std::string &query, const mm_reg1_t *reg, query_size_type ldist,
            std::unique_ptr<std::string>&& cs)
        : Mapped(query, reg)
        , ldist_(ldist)
        , qlen_(query.size())
    {
        auto n = query.size();
        if (true)
            cs_ = std::move(cs);
        if (reg->qs != 0)
            qs_str_ = std::make_unique<std::string>(query, 0, reg->qs);
        if (reg->qe != qlen_)
            qe_str_ = std::make_unique<std::string>(query, reg->qe, qlen_ - reg->qe);
    }

    Aligned(Aligned&&) = default;
    Aligned& operator=(Aligned&&) = default;

    size_t ldist() const { return ldist_; }
    size_t qlen() const { return qlen_; }

    size_t qs() const { return qs_str_? qs_str_->size() : 0; }
    size_t qe() const { return qe_str_? qe_str_->size() : 0; }
    const std::string* cs() const { return cs_.get(); }
    const std::string* qs_str() const { return qs_str_? qs_str_.get() : &empty_string_; }
    const std::string* qe_str() const { return qe_str_? qe_str_.get() : &empty_string_; }

};
#pragma pack(pop)


class AlignerBase: public MapperBase {

    private:
	std::basic_string<enc::encoded> ref_buf_;
	std::basic_string<enc::encoded> query_buf_;

    protected:
	using MapperBase::MapperBase;

	const auto ref_enc() const { return ref_buf_.data(); }
	const auto query_enc() const { return query_buf_.data(); }

	// Encode the whole query into the query buffer 
	void load_query(std::string const& q, bool reverse_complement) {
		query_buf_.reserve(q.size());
		enc::encode_str(&query_buf_[0],
				q.data(),
				q.data() + q.size(),
				reverse_complement);
	}

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
            load_query(query, hits[0].rev);
            load_ref(hits[0].rid, hits[0].rs, hits[0].re);
            CsWriter csw(ref_enc(), query_enc(), hits[0].p);
            unsigned char ldist = csw.align();
            auto csp = csw.get_cs();
            return Aligned<>(query, hits.get(), ldist, std::move(csp));
        } else {
            return Aligned<>();
        }
    }
};


} // namespace mm
