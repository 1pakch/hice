#ifndef HC_PAIRS
#define HC_PAIRS

#include "minimap.h"
#include "hc_fastq.h"

typedef struct {
	char *data;
	uint16_t len;
} hc_str_t;

typedef struct {
	hc_str_t first;
	hc_str_t second;
} hc_read_pair_t;

typedef struct {
	mm_reg1_t* first;
	mm_reg1_t* second;
} hc_mapped_pair_t;

hc_str_t hc_str(kstring_t s)
{
	hc_str_t res;
	res.len = s.l;
	res.data = (char*) malloc(res.len);
	memcpy(res.data, s.s, res.len);
	return res;
}

hc_read_pair_t hc_read_pair(kseq_t read1, kseq_t read2)
{
	hc_read_pair_t result;
	result.first = hc_str(read1.seq);
	result.second = hc_str(read2.seq);
	return result;
}

#endif
