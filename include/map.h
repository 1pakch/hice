#pragma once

#include <minimap.h>
#include "hc/mem.h"

typedef struct {
	mm_idx_t *idx;
	mm_idxopt_t *idxopt;
	mm_mapopt_t *mapopt;
	mm_tbuf_t *tbuf;
	size_t max_read_length;
} hc_map_ctx_t;

hc_map_ctx_t* hc_map_ctx_init_global(const char* ref_path, size_t max_read_length,
				size_t n_threads, int verbose)
	mm_verbose = verbose;
	hc_map_ctx_t* ctx = MALLOC(hc_map_ctx_t);
	ctx->tbuf = NULL;
	ctx->max_read_length = max_read_length;
	ctx->idxopt = MALLOC(mm_idxopt_t);
	ctx->mapopt = MALLOC(mm_mapopt_t);
	cxt->idx = mm_idx_read(ref_path, &ctx->idxopt, n_threads);
	if (!ctx->idx) {
		FREE(ctx->idxopt);
		FREE(ctx->mapopt);
		return NULL;
	}
	mm_set_opt(0, &iopt, &mopt);
	ctx->mapopt->flag |= MM_F_CIGAR; // Perform alignment
	ctx->mapopt->flag |= MM_F_SR;    // Short reads profile
	// ctx->mopt.end_bonus = 20;
	mm_mapopt_update(ctx->mapopt, ctx->idx); // this sets the maximum minimizer occurrence;
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
		mm_idx_destroy(ctx->idx);
		FREE(ctx->idxopt);
		FREE(ctx->mapopt);
		FREE(ctx);
	}
}

hc_mapped_t hc_map(const char *seqstr, size_t seqlen, hc_map_ctx_t *map_ctx, hc_aln_ctx_t *aln_ctx)
{
	hc_mapped_t *x = malloc(sizeof(hc_mapped_t));
	int n_hits;
	mm_reg1_t *hits;
	hits = mm_map(map_ctx->idx, seqlen, seqstr, &n_hits, map_ctx->tbuf, map_ctx->mapopt, 0);
	if (n_hits) {
		*x = hc_mapped_init(hits, NULL);
		hc_alignment_t details = hc_align(seqstr, seqlen, map_ctx->idx, aln_ctx);
		free(hits->p);
		free(hits);
		if (details.qs || details.qe || details.ldistance) {
			x->details = (hc_alignment_t *) malloc(sizeof(hc_alignment_t));
			*(x->details) = details;
		}
	} else {
		*x = hc_mapped_empty();
	}
	return *x;
}
