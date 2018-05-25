// settings.cpp - minimap2 settings and index loading/construction

#pragma once

#include <minimap.h>
#include <include/mm2xx/handles.hpp>


namespace mm {


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


class Settings {
    private:
	handle<mm_idx_t> idx_;
	handle<mm_idxopt_t> idxopt_;
	handle<mm_mapopt_t> mapopt_;

    public:
	// nullptr = default preset
	Settings(const char* preset=nullptr)
		: idxopt_(new mm_idxopt_t()) // init with zeros
		, mapopt_(new mm_mapopt_t()) // init with zeros
	{
		mm_set_opt(preset, idxopt_.get(), mapopt_.get());

		// Without this flag mm tends to produce alignments where
		// query starts and ends are omitted from the alignment
		// (even if they match the reference perfectly). Hence
		// we turn it on even if we do not need alignment info.
		mapopt_->flag |= MM_F_CIGAR;

		// This option is supposed to increase propensity of mm
		// to include the beginnings/ends in the alignment. It
		// doesn't seem to help much with the "sr" preset though.
		// See https://github.com/lh3/minimap2/issues/149
		//
		// mapopt_.get()->end_bonus = 20;
	}

	const mm_mapopt_t* mapopt() const { return mapopt_.get(); }
	const mm_idxopt_t* idxopt() const { return idxopt_.get(); }
	const mm_idx_t* idx() const { return idx_.get(); }

	void set_reference(const char* path, size_t n_threads=4) {
		idx_ = std::move(mm_idx_read1(path, idxopt(), n_threads));
		assert(idx_);
		mm_mapopt_update(mapopt_.get(), idx_.get());
	}
};

} // mm
