#pragma once

#include <cstdlib>
#include <cassert>
#include <memory>
#include <vector>
#include <ostream>

#include <minimap.h>
#include <hice/hc_pair_types.h>
#include <hice/hc_align.h>

namespace mapping {

// Define deleter structs for unique_ptr's wrapping mimimap2 pointer types
namespace deleters {

// Allocations handled by C++ should use new
template<typename T>
struct deleter: public std::default_delete<T> {};

// Pointers returned by minimap2 should use be freed accordingly
template<>
struct deleter<mm_idx_t> {
	void operator()(mm_idx_t *p){ mm_idx_destroy(p); }
};

template<>
struct deleter<mm_tbuf_t> {
	void operator()(mm_tbuf_t *p){ mm_tbuf_destroy(p); }
};

template<>
struct deleter<mm_reg1_t> {
	void operator()(mm_reg1_t *p) {
		std::free(p);
	}
};

struct free {
	void operator()(void *p) {
		std::free(p);
	}
};

} // deleters

class CigarOp {
	int* p_;
	CigarOp(int* cigar_int): p_(cigar_int) {}
	bool length() const { return p_ && 0xf; }
};

struct Cigar: public mm_extra_t {
	auto size() const { return n_cigar; }
};


struct Alignment: public mm_reg1_t {
	Alignment(mm_reg1_t reg): mm_reg1_t(reg) {}
	~Alignment(){
		std::free(this->p);
	}
	friend std::ostream& operator<<(std::ostream& o, const Alignment& a) {
		o << a.rid << a.rs << a.re << "+-"[a.rev] << a.mapq << '\n';
		return o;
	};
};

//! unique_ptr wrapping minimap2 pointer types with approriate deleters
template<typename T>
using handle = std::unique_ptr<T, deleters::deleter<T>>;

template<typename T, typename Deleter=deleters::free>
class array_handle: private std::unique_ptr<T, Deleter> {
    private:
	using Base = std::unique_ptr<T, Deleter>;
	size_t size_; // to be ignored if base pointer is null

    public:
	array_handle() {} // base pointer is null-initialized
	
	array_handle(T *p, size_t size): Base(p), size_(size) {}
	
	array_handle(array_handle&& v): Base(std::move(v)), size_(v.size_) {}

	array_handle& operator=(array_handle&& v) {
		Base::operator=(std::move(v));
		size_ = v.size_;
		return *this;
	}

	~array_handle() {
		// call items' destructors if non-empty
		for (auto p=begin(); p < end(); ++p) p->~T();
	}

	size_t empty() const { return !this->get(); }
	size_t size() const { return empty() ? 0 : size_; }

	T* begin() { return this->get(); }
	T* end() { return this->get() + size(); }
	const T* cbegin() const { return begin(); }
	const T* cend() const { return end(); }
};


//! Read an index containing 1 part at maximum
handle<mm_idx_t> mm_idx_read1(const char* fname, const mm_idxopt_t* iopt,
			      size_t n_indexing_threads)
{
	mm_idx_reader_t *r = mm_idx_reader_open(fname, iopt, 0);
	if (!r) {
		fprintf(stderr, "mm_idx_read(): error opening %s", fname);
		return nullptr;
	}
	if (r->n_parts > 1) {
		fprintf(stderr, "mm_idx_read(): n_parts > 1");
		return nullptr;
	}
	mm_idx_t *idx = mm_idx_reader_read(r, n_indexing_threads);
	mm_idx_reader_close(r);
	if (idx->n_seq >= UINT8_MAX) {
		fprintf(stderr, "mm_idx_read(): n_seq >= %d", UINT8_MAX);
		mm_idx_destroy(idx);
		return nullptr;
	}
	return handle<mm_idx_t>(idx);
}

class Mapper;

class Settings {
    protected:
	friend class Mapper;
	handle<mm_idx_t> idx_;
	handle<mm_idxopt_t> idxopt_;
	handle<mm_mapopt_t> mapopt_;
    public:

	// nullptr = default preset
	Settings(const char* reference_path, const char* preset=nullptr,
		 size_t n_indexing_threads=4)
		: idxopt_(new mm_idxopt_t) 
		, mapopt_(new mm_mapopt_t)
	{
		mm_set_opt(preset, idxopt_.get(), mapopt_.get());
		idx_ = std::move(mm_idx_read1(reference_path,
					      idxopt_.get(),
					      n_indexing_threads));
		assert(idx_);
		mm_mapopt_update(mapopt_.get(), idx_.get());
	}

	void set_mapopt_flags(int flags) { mapopt_->flag |= flags; }

	Mapper create_mapper(); 
};

class Mapper {
    private:
	const Settings& s_;
	mm_mapopt_t mapopt_;
	handle<mm_tbuf_t> thread_buf_;
	std::string template_buf_;
	std::string query_buf_;

    public:
	Mapper(const Settings& settings)
		: s_(settings)
		, thread_buf_(mm_tbuf_init())
	{
		mapopt_ = *s_.mapopt_.get();
		assert(thread_buf_.get());
	}

	const mm_mapopt_t* mapopt() const { return &mapopt_; }
	const mm_idxopt_t* idxopt() const { return s_.idxopt_.get(); }
	const mm_idx_t* idx() const { return s_.idx_.get(); }

	array_handle<Alignment> map(std::string const& read, unsigned char n_retain) {
		mapopt_.best_n = (int) n_retain;
		int n_hits;
		Alignment* hits = reinterpret_cast<Alignment*>(_map(read, &n_hits));
		//if (mapopt()->flag && MM_F_CIGAR) 
		return std::move(array_handle<Alignment>(hits, n_hits));
	}

	handle<Alignment> map1(std::string const& read) {
		mapopt_.best_n = 1;
		int n_hits;
		Alignment* hit = reinterpret_cast<Alignment*>(_map(read, &n_hits));
		//if (mapopt()->flag && MM_F_CIGAR) 
		return std::move(handle<Alignment>(hit));
	}

    private:

	mm_reg1_t* _map(std::string const& read, int *n_hits) {
		return mm_map(idx(), read.size(), read.c_str(), n_hits,
			      thread_buf_.get(), mapopt(), nullptr);
	}

	//! Add CIGAR information to the generated mappings
	void fill_cigar(std::string const& read, Alignment* al) {
		template_buf_.reserve(read.size());
		query_buf_.reserve(read.size());
		mm_idx_getseq(idx(), al->rid, al->rs, al->re, const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(template_buf_.data())));
	}
};

Mapper Settings::create_mapper() { return Mapper(*this); } 

} // mm

