// hc_athreads.h - aligner threads

#ifndef HC_ATHREAD
#define HC_ATHREAD

#include <string.h>

#include "minimap.h"
#include "kseq.h"

#include "hc_pair_types.h"
#include "hc_mm_utils.h"


typedef struct {
	mm_idx_t *idx;
	mm_mapopt_t *mapopt;
	pipe_consumer_t* src;
	pipe_producer_t* dst;
	size_t bufsize;
	pthread_t thread;
} hc_athread_t;


mm_extra_t* hc_mm_copy_extra(mm_extra_t* src)
{
	if (!src) return NULL;
	mm_extra_t* dst = (mm_extra_t*) malloc(sizeof(mm_extra_t*));
	*dst = *src;
	uint32_t n_cigar = src->n_cigar;
	dst->capacity = n_cigar;
	memcpy(dst->cigar, src->cigar, n_cigar*sizeof(uint32_t));
	return dst;
}

mm_reg1_t* hc_mm_best_hit(hc_athread_t *ctx, hc_str_t seq, mm_tbuf_t *tbuf)
{
	mm_reg1_t *hits, *best;
	int n_hits;
	hits = mm_map(ctx->idx, seq.len, seq.data, &n_hits, tbuf, ctx->mapopt, 0);
	if (n_hits) {
		// copy first (best) hit
		//best = (mm_reg1_t*) malloc(sizeof(mm_reg1_t));
		//*best = *hits;
		//if (hits->p) best->p = hc_mm_copy_extra(hits->p);
		// free original hits
		//for (int i=0; i<n_hits; ++i) free(hits[i].p);
		//free(hits);
		return hits;
	} else {
		// return a hit with rid = UINT8_MAX if no hits
		best = (mm_reg1_t*) calloc(1, sizeof(mm_reg1_t));
		best->rid = UINT8_MAX;
	}
	return best;
}

void* hc_athread_entry(void *ctx_)
{
	hc_athread_t *ctx = (hc_athread_t*) ctx_;
	mm_tbuf_t *tbuf =  mm_tbuf_init(); // thread buffer
	hc_read_pair_t *inbuf = (hc_read_pair_t*) malloc(ctx->bufsize*sizeof(hc_read_pair_t));
	size_t n;
	while ((n = pipe_pop(ctx->src, inbuf, ctx->bufsize))) {
		hc_mapped_pair_t* outbuf = (hc_mapped_pair_t*) malloc(n*sizeof(hc_mapped_pair_t));
		for (int i=0; i < n; ++i) {
			outbuf[i].first = hc_mm_best_hit(ctx, inbuf[i].first, tbuf);
			outbuf[i].second = hc_mm_best_hit(ctx, inbuf[i].second, tbuf);
			free(inbuf[i].first.data);
			free(inbuf[i].second.data);
		}
		pipe_push(ctx->dst, outbuf, n);
	}
	mm_tbuf_destroy(tbuf);
	free(inbuf);
	return NULL;
}

hc_athread_t* hc_athread_create(mm_idx_t *idx, mm_mapopt_t *mapopt, pipe_t *src, pipe_t *dst)
{
	hc_athread_t *ctx = (hc_athread_t*) malloc(sizeof(hc_athread_t));
	ctx->idx = idx;
	ctx->mapopt = mapopt;
	ctx->src = pipe_consumer_new(src);
	ctx->dst = pipe_producer_new(dst);
	ctx->bufsize = 10;
	pthread_create(&ctx->thread, NULL, hc_athread_entry, ctx);
	return ctx;
}

void hc_athread_join(hc_athread_t *ctx)
{
	pthread_join(ctx->thread, NULL);
	pipe_consumer_free(ctx->src);
	pipe_producer_free(ctx->dst);
	free(ctx);
}


#endif
