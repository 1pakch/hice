// hc_rthread.h - a thread reading paired fastq files

#ifndef HC_RTHREAD
#define HC_RTHREAD

#include <string.h>
#include "pipe.h"
#include "minimap.h"

#include "hc_pair_types.h"
#include "hc_mm_utils.h"
#include "hc_fastq.h"


typedef struct {
	char **fnames;
	hc_fastq_parser_t parsers[2];
	pipe_producer_t* dst;
	size_t bufsize;
	pthread_t thread;
} hc_rthread_t;

void* hc_rthread_entry(void *ctx_)
{
	hc_rthread_t *ctx = (hc_rthread_t*) ctx_;
	kseq_t* read1=(ctx->parsers[0].current_read);
	kseq_t* read2=(ctx->parsers[1].current_read);
	hc_read_pair_t buf[ctx->bufsize];
	int pos=0;
	while (hc_fastq_read_paired(ctx->parsers)) {
		if (pos < ctx->bufsize) {
			buf[pos] = hc_read_pair(*read1, *read2);
			++pos;
		} else {
			pipe_push(ctx->dst, buf, pos);
			pos = 0;
		}
	}
	if (pos) pipe_push(ctx->dst, buf, pos);
	return NULL;
}

hc_rthread_t* hc_rthread_create(char **fnames, pipe_t* dst, size_t bufsize)
{
	hc_rthread_t *ctx = (hc_rthread_t*) malloc(sizeof(hc_rthread_t));
	if (!hc_fastq_open_many(2, fnames, ctx->parsers)) {
		ctx->dst = pipe_producer_new(dst);
		ctx->bufsize = bufsize;
		pthread_create(&ctx->thread, NULL, hc_rthread_entry, ctx);
		return ctx;
	} else {
		return NULL;
	}
}

void hc_rthread_join(hc_rthread_t *ctx)
{
	pthread_join(ctx->thread, NULL);
	pipe_producer_free(ctx->dst);
	hc_fastq_close_many(2, ctx->parsers);
	free(ctx);
}


#endif
