// hc_athreads.h - aligner threads

#ifndef HC_ATHREAD
#define HC_ATHREAD

#include <string.h>

#include "minimap.h"
#include "kseq.h"

#include "map.h"
#include "hc_pair_types.h"
#include "hc_mm_utils.h"
#include "hc_align.h"


typedef struct {
	hc_map_ctx_t* map_ctx;
	hc_aln_xtx_t* aln_ctx;
	pipe_consumer_t* src;
	pipe_producer_t* dst;
	size_t bufsize;
	pthread_t thread;
	hc_cs_buf_t *csbuf;
} hc_athread_t;


void* hc_athread_entry(void *ctx_)
{
	hc_athread_t *ctx = (hc_athread_t*) ctx_;
	hc_read_pair_t *inbuf = (hc_read_pair_t*) malloc(ctx->bufsize*sizeof(hc_read_pair_t));
	size_t n;
	while ((n = pipe_pop(ctx->src, inbuf, ctx->bufsize))) {
		hc_mapped_pair_t* outbuf = (hc_mapped_pair_t*) malloc(n*sizeof(hc_mapped_pair_t));
		for (int i=0; i < n; ++i) {
			outbuf[i].first = hc_mm_map(ctx, inbuf[i].first);
			outbuf[i].second = hc_mm_map(ctx, inbuf[i].second);
			free(inbuf[i].first.data);
			free(inbuf[i].second.data);
		}
		pipe_push(ctx->dst, outbuf, n);
	}
	free(inbuf);
	return NULL;
}

hc_athread_t* hc_athread_create(mm_idx_t *idx, mm_mapopt_t *mapopt, pipe_t *src,
				pipe_t *dst, size_t bufsize)
{
	hc_athread_t *ctx = (hc_athread_t*) malloc(sizeof(hc_athread_t));
	ctx->map_ctx = (hc_map_ctx_t *) malloc(sizeof(hc_map_ctx_t));
	*ctx->map_ctx = hc_map_ctx_t {

	};
	ctx->src = pipe_consumer_new(src);
	ctx->dst = pipe_producer_new(dst);
	ctx->bufsize = bufsize;
	pthread_create(&ctx->thread, NULL, hc_athread_entry, ctx);
	return ctx;
}

void hc_athread_join(hc_athread_t *ctx)
{
	pthread_join(ctx->thread, NULL);
	mm_tbuf_destroy(ctx->tbuf);
	hc_cs_buf_destroy(ctx->csbuf);
	pipe_consumer_free(ctx->src);
	pipe_producer_free(ctx->dst);
	free(ctx);
}


#endif
