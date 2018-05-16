
#include <minimap.h>

#include "hice/map_ctx.h"


mm_idx_t* mm_idx_read(const char* fname, const mm_idxopt_t* iopt,
		      size_t n_threads)
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
	mm_idx_t *idx = mm_idx_reader_read(r, n_threads);
	mm_idx_reader_close(r);
	if (idx->n_seq >= UINT8_MAX) {
		fprintf(stderr, "mm_idx_read(): n_seq >= %d", UINT8_MAX);
		mm_idx_destroy(idx);
		return NULL;
	}
	return idx;
}


hc_map_ctx_t* hc_map_ctx_init_global(const char* reference_path,
				     size_t max_read_length,
				     size_t n_indexing_threads,
				     int verbose)
{
	mm_verbose = verbose;
	hc_map_ctx_t* ctx = MALLOC(hc_map_ctx_t);
	ctx->tbuf = NULL;
	ctx->max_read_length = max_read_length;
	ctx->idxopt = MALLOC(mm_idxopt_t);
	ctx->mapopt = MALLOC(mm_mapopt_t);
	mm_set_opt(0, ctx->idxopt, ctx->mapopt);
	ctx->idx = mm_idx_read(reference_path, ctx->idxopt, n_indexing_threads);
	if (!ctx->idx) {
		FREE(ctx->idxopt);
		FREE(ctx->mapopt);
		return NULL;
	}
	mm_mapopt_update(ctx->mapopt, ctx->idx); 
	ctx->mapopt->flag |= MM_F_CIGAR; // Perform alignment
	ctx->mapopt->flag |= MM_F_SR;    // Short reads profile
	// ctx->mopt.end_bonus = 20;
	return ctx;
}

hc_map_ctx_t* hc_map_ctx_init_thread(const hc_map_ctx_t* global)
{
	hc_map_ctx_t* ctx = MALLOC(hc_map_ctx_t);
	*ctx = *global;
	ctx->tbuf = mm_tbuf_init();
	return ctx;
}

void hc_map_ctx_free(hc_map_ctx_t* map_ctx)
{
	if (map_ctx->tbuf) {
		// Thread local context
		mm_tbuf_destroy(map_ctx->tbuf);
		FREE(map_ctx);
	} else {
		// Global context
		mm_idx_destroy(map_ctx->idx);
		FREE(map_ctx->idxopt);
		FREE(map_ctx->mapopt);
		FREE(map_ctx);
	}
}
