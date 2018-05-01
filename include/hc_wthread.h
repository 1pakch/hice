// hc_wthread.h - a thread writing mapped pairs

#ifndef HC_WTHREAD
#define HC_WTHREAD

#include "pipe.h"
#include "minimap.h"

#include "hc_pair_types.h"
#include "hc_mm_utils.h"
#include "hc_fastq.h"


typedef struct {
	pipe_consumer_t* src;
	//gsFile dst;
	size_t bufsize;
	pthread_t thread;
} hc_wthread_t;


static inline void print_alignment2(mm_reg1_t *r)
{
	printf("%d \t %d \t %d \t %c \t %d \t %d \t %d \t %d \t %d\n",
		r->rid, r->rs, r->re, "+-"[r->rev], r->qs, r->qe,
		r->mlen, r->blen, r->mapq);
}

void* hc_wthread_entry(void *ctx_)
{
	hc_wthread_t *ctx = (hc_wthread_t*) ctx_;
	hc_mapped_pair_t* inbuf = (hc_mapped_pair_t*) malloc(sizeof(hc_mapped_pair_t)*ctx->bufsize);
	size_t n;

	while ((n = pipe_pop(ctx->src, inbuf, ctx->bufsize))) {
		for (int i=0; i<n; ++i) {
			print_alignment2(inbuf[i].first);
			print_alignment2(inbuf[i].second);
			free(inbuf[i].first->p);
			free(inbuf[i].second->p);
			free(inbuf[i].first);
			free(inbuf[i].second);
		}
	}
	return NULL;
}

hc_wthread_t* hc_wthread_create(pipe_t* src)
{
	hc_wthread_t *ctx = (hc_wthread_t*) malloc(sizeof(hc_wthread_t));
	ctx->src = pipe_consumer_new(src);
	ctx->bufsize = 1000;
	pthread_create(&ctx->thread, NULL, hc_wthread_entry, ctx);
	return ctx;
}

void hc_wthread_join(hc_wthread_t *ctx)
{
	pthread_join(ctx->thread, NULL);
	pipe_consumer_free(ctx->src);
	free(ctx);
	
}


#endif
