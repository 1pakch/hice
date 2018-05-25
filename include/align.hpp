#include <string>
#include <sstream>
#include <cstdint>

#include <minimap2/minimap.h>
#include <include/minimap.hpp>
#include <include/mm2xx/encoding.hpp>
#include <include/mm2xx/cswriter.hpp>


extern uint8_t seq_nt4_table[256]; // encoding from 'ACGT' to internal

namespace mm {


template<typename query_size_t=uint8_t>
struct Aligned {
	std::unique_ptr<std::string> cs_;     // cs tag if ldist_ > 0
	std::unique_ptr<std::string> qstr_start_; // query_string[:qs_]
	std::unique_ptr<std::string> qstr_end_;   // query_string[(qs_+qlen_):]
	query_size_t ldist_;    // levenshtein distance
	Aligned(): ldist_(0) {}
	Aligned(query_size_t ldist, std::unique_ptr<std::string>&& cs): cs_(std::move(cs)), ldist_(ldist) {}
};


class AlignerBase: protected MapperBase {

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
		enc::encode_str(query_buf_.data(),
				q.data(),
				q.data() + q.size(),
				reverse_complement);
	}

	// Encode the whole query into the query buffer 
	void load_ref(uint8_t rid, uint32_t start, uint32_t end) {
		ref_buf_.reserve(end - start);
		mm_idx_getseq(idx(), rid, start, end, &ref_buf_.data()[0]);
	}

};


/// Aligner returning the best alignment for a given query
class Aligner: public AlignerBase {
    public:
	Aligner(const Settings& settings): AlignerBase(settings, 1) {}
	
	Aligned<>* align(std::string const& query, const mm_reg1_t* hit) {
		load_query(query, hit->rev);
		load_ref(hit->rid, hit->rs, hit->re);
		CsWriter csw(ref_enc(), query_enc(), hit->p);
		size_t ldist = csw.align();
		auto csp = csw.get_cs();
		if (!ldist) return nullptr;
		else return new Aligned(ldist, std::move(csp));
	}
};


/// Aligner returning up to n_max alignments for a given query
class MultiAligner: public AlignerBase {
    public:
	MultiAligner(const Settings& settings, uint8_t n_max)
		: AlignerBase(settings, n_max)
	{}
	
	std::vector<Aligned<>> align(std::string const& read, const mm_reg1_t* hit) {
	}
};


} // namespace mm
