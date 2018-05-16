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



static inline void print_alignment2(hc_mapped_t r)
{
	static const char no_tag[] = "<none>";
	const char *csstr = r.details && r.details->alstr ? r.details->alstr->s : no_tag;
	printf("%d \t %d \t %d \t %c \t %u \t %d \t %s \n",
		r.rid1,	// index of the reference sequence
		r.rs, 		// reference start (0-based, closed)
		r.re, 		// reference end   (0-based, open)
		"+-"[r.rev],	// strand
		//r->qs, 		// query start (0-based, closed)
		//r->qe, 		// query end (0-based, open)
		//r->mlen,	// seeded exact match length
		r.primary,	// seeded alignment block length
		r.mapq,	// MAPQ (<=60)
		csstr);
}

void* hc_wthread_entry(void *ctx_)
{
	hc_wthread_t *ctx = (hc_wthread_t*) ctx_;
	hc_mapped_pair_t* inbuf = (hc_mapped_pair_t*) malloc(sizeof(hc_mapped_pair_t)*ctx->bufsize);
	size_t n;

	//printf("%d\n", sizeof(hc_aln8_t));
	//printf("%d\n", sizeof(mm_reg1_t));
	while ((n = pipe_pop(ctx->src, inbuf, ctx->bufsize))) {
		for (int i=0; i<n; ++i) {
			print_alignment2(inbuf[i].first);
			print_alignment2(inbuf[i].second);
			/*
			if (inbuf[i].first.details) {
				free(cs1->s);
				free(cs1);
			}
			if (cs2) {
				free(cs2->s);
				free(cs2);
			}
			free(inbuf[i].first);
			free(inbuf[i].second);
			*/
		}
	}
	return NULL;
}

hc_wthread_t* hc_wthread_create(pipe_t* src, size_t bufsize)
{
	hc_wthread_t *ctx = (hc_wthread_t*) malloc(sizeof(hc_wthread_t));
	ctx->src = pipe_consumer_new(src);
	ctx->bufsize = bufsize;
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
