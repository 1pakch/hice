// hc_mm_utils.h - some utility functions around minimap2 API

#ifndef HC_MM_UTILS
#define HC_MM_UTILS

#include <string.h>

#include "minimap.h"


mm_idx_t* mm_idx_read(char* fname, const mm_idxopt_t* iopt, size_t n_threads)
{
	mm_idx_reader_t *r = mm_idx_reader_open(fname, iopt, 0);
	if (!r) {
		fprintf(stderr, "mm_idx_read(): error opening %s", fname);
		return NULL;
	}
	if (r->n_parts > 1) {
		fprintf(stderr, "mm_idx_read(): n_parts > 1");
		return NULL;
	}
	mm_idx_t *mi = mm_idx_reader_read(r, n_threads);
	mm_idx_reader_close(r);
	if (mi->n_seq >= UINT8_MAX) {
		fprintf(stderr, "mm_idx_read(): n_seq >= %d", UINT8_MAX);
		mm_idx_destroy(mi);
		return NULL;
	}
	return mi;
}

#endif
