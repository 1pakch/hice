#ifndef HC_FASTQ
#define HC_FASTQ

#include <zlib.h>

#include "kseq.h"
#include "pipe.h"


KSEQ_INIT(gzFile, gzread)


typedef struct {
	gzFile file;
	kseq_t *current_read;
} hc_fastq_parser_t;


int hc_fastq_open(char* fname, hc_fastq_parser_t *parser)
{
	if (!(parser->file = gzopen(fname, "r"))) {
		fprintf(stderr, "gzopen(): cannot open %s\n", fname);
		return 1;
	}
	if (!(parser->current_read = kseq_init(parser->file))) {
		fprintf(stderr, "kseq_init(): error when opening %s\n", fname);
		return 2;
	}
	return 0;
}

static inline int hc_fastq_next(hc_fastq_parser_t parser)
{
	return kseq_read(parser.current_read) >= 0;
}

void hc_fastq_close(hc_fastq_parser_t parser)
{
	kseq_destroy(parser.current_read);
	gzclose(parser.file);
}

int hc_fastq_open_many(size_t n, char* fnames[], hc_fastq_parser_t parsers[])
{
	int i, err = 0;
	for (i=0; !err && i<n; ++i) {
		err = hc_fastq_open(fnames[i], &parsers[i]);
	}
	return err;
}

int hc_fastq_read_paired(hc_fastq_parser_t parsers[])
{
	int ok = 1;
	for (int i=0; ok && i<2; ++i) {
		ok = ok &&  hc_fastq_next(parsers[i]);
	}
	return ok;
}

void hc_fastq_close_many(size_t n, hc_fastq_parser_t parsers[])
{
	for (int i=0; i<n; ++i) {
		hc_fastq_close(parsers[i]);
	}
}

#endif
