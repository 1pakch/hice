#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>

#include <minimap.h>
#include <include/mm2xx/settings.hpp>
#include <include/mm2xx/handles.hpp>


namespace mm {


#pragma pack(push,1)


const std::string empty_string_;

template<typename query_size_t=uint8_t,
	 typename genome_index_t=uint8_t,
	 typename reference_size_t=uint32_t>
class Mapped {
	genome_index_t rid1_;   // 1-based chromosome index

	reference_size_t rs_;   // start of the alignment in the reference
	query_size_t rlen_;     // length of the alignment in the reference

	query_size_t qstr_len_; // length of the query string
	query_size_t ldist_;    // Levenshtein distance

	uint8_t mapq_    :6,    // mapping quality
		rc_      :1,    // if the query is reverse complimented in the alignment
		primary_ :1;    // if the alignment was marked as primary

	//std::unique_ptr<Aligned<query_size_t>> aligned_; // details 

    public:
	genome_index_t rid1() const { return rid1_; }
	bool is_mapped() const { return rid1(); }
	query_size_t rstart() const { return rs_; }
	query_size_t rend() const { return rs_ + rlen_; }
	query_size_t qlen() const { return qstr_len_; }
	uint8_t mapq() const { return mapq_; }
	bool is_rc() const { return rc_; }
	bool is_primary() const { return primary_; }

	/*
	const std::string& cs() const {
		if (aligned_ && aligned_->ldist_)
			return *(aligned_->cs_.get());
		else
			return mm::empty_string_; 
	}
	*/

	Mapped(): rid1_(0) {}
	Mapped(const std::string& query,
	       const mm_reg1_t* reg)
	: rid1_(reg->rid+1)
	, rs_(reg->rs)
	, rlen_(reg->re - reg->rs)
	, qstr_len_(query.size())
  	, mapq_(reg->mapq)
	, rc_(reg->rev)
	, primary_(reg->id == reg->parent)
	//, aligned_(std::move(aligned))
	{}


};


#pragma pack(pop)






class MapperBase {
   private:
	const mm_idx_t* idx_;
	mm_mapopt_t mapopt_;
	handle<mm_tbuf_t> thread_buf_;

    protected: 
	MapperBase(const Settings& settings, uint8_t n_max)
		: idx_(settings.idx())
		, mapopt_(*settings.mapopt())
		, thread_buf_(mm_tbuf_init())
	{
		assert(idx_);
		assert(mapopt_.flag & MM_F_CIGAR);
		assert(thread_buf_.get());
		assert(n_max > 0);
		mapopt_.best_n = n_max;
	}
	
	const mm_mapopt_t* mapopt() const { return &mapopt_; }
	const mm_idx_t* idx() const { return idx_; }
	
	handle<mm_reg1_t[]> _map(std::string const& query) {
		int n_hits;
		mm_reg1_t* hits = mm_map(idx(),
					 query.size(),
					 query.c_str(),
					 &n_hits,
		       			 thread_buf_.get(),
					 mapopt(),
					 nullptr);
		assert(n_hits >= 0);
		handle<mm_reg1_t[]> result(hits, static_cast<size_t>(n_hits));
		return std::move(result);
	}
	
};


/// Mapper returning the best alignment for a given query
class Mapper: protected MapperBase {

    public:
	Mapper(const Settings& settings): MapperBase(settings, 1) {}

	Mapped<> map(std::string const& query) {
		auto hits = _map(query);
		return std::move(Mapped<>(query, hits.get()));
	}

};

/// Mapper returning up to n_max alignments for a query
class MultiMapper: protected MapperBase {

    public:
	MultiMapper(const Settings& settings, uint8_t n_max)
		: MapperBase(settings, n_max)
	{}

	std::vector<Mapped<>> map(std::string const& query) {
		/*
		int n_hits;
		mm_reg1_t *hits = _map(query, &n_hits);
		std::vector<Mapped<>> result;
		for (int i=0; i < n_hits; ++i) {
			result.emplace_back(Mapped<>(query, hits + i));
		}
		return std::move(result);
		*/
	}

};


} // mm

