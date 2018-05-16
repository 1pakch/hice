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
	kstring_t* qs;		// skipped characters before the query start (never RCd)
	kstring_t* qe;		// skipped characters after query end (never RCd)
	kstring_t* alstr;	// alignment string (cs tag) if ldistance > 0
	uint16_t ldistance;	// Levenshtein distance between the possibly RCd query and the ref
} hc_alignment_t;

typedef struct {
	hc_alignment_t *details; // pointer to alignment details if ldistance > 0 or query sequence was trimmed
	int32_t rs, re;  	 // reference start and end
	uint8_t rid1;    	 // 1-based reference index. zero if unmapped
	uint8_t mapq:6,	 
		rev:1,		 // if the query is reverse complimented in the alignment
		primary:1;
} hc_mapped_t;

typedef struct {
	hc_mapped_t first;
	hc_mapped_t second;
} hc_mapped_pair_t;

hc_str_t hc_str(kstring_t s)
{
	hc_str_t res;
	res.len = s.l;
	res.data = (char*) malloc(res.len);
	memcpy(res.data, s.s, res.len);
	return res;
}


hc_mapped_t hc_mapped_empty()
{
	hc_mapped_t result = {NULL, 0, 0, 0, 0, 0};
	return result;
}

hc_mapped_t hc_mapped_init(mm_reg1_t *r, hc_alignment_t *details)
{
	hc_mapped_t result = {
		.rs = r->rs,
		.re = r->re,
		.rid1 = r->rid+1,
		.rev = r->rev,
		.primary = r->parent == r->id,
		.mapq = r->mapq
	};
	return result;
}

hc_read_pair_t hc_read_pair(kseq_t read1, kseq_t read2)
{
	hc_read_pair_t result;
	result.first = hc_str(read1.seq);
	result.second = hc_str(read2.seq);
	return result;
}

#endif
