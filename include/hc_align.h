#pragma once

#include "kseq.h"


typedef struct {
	uint8_t *qseq;
	uint8_t *tseq;
	char *tmp;
} hc_aln_ctx_t;


hc_cs_buf_t* hc_aln_ctx_init(size_t maxlength) {
	hc_aln_ctx_t * aln_ctx = (aln_ctx_t *) malloc(sizeof(hc_aln_ctx_t));
	aln_ctx->qseq = (uint8_t *) malloc(maxlength);
	aln_ctx->tseq = (uint8_t *) malloc(maxlength);
	aln_ctx->tmp = (char *) malloc(maxlength*4);
	return aln_ctx;
}

void hc_aln_ctx_destroy(hc_aln_ctx_t* aln_ctx) {
	free(aln_ctx->tmp);
	free(aln_ctx->qseq);
	free(aln_ctx->tseq);
	free(aln_ctx);
}


static inline void str_enlarge(kstring_t *s, int l)
{
	if (s->l + l + 1 > s->m) {
		s->m = s->l + l + 1;
		kroundup32(s->m);
		s->s = (char*)realloc(s->s, s->m);
	}
}

static inline void str_copy(kstring_t *s, const char *st, const char *en)
{
	str_enlarge(s, en - st);
	memcpy(&s->s[s->l], st, en - st);
	s->l += en - st;
}

void hc_sprintf(kstring_t *s, const char *fmt, ...)
{
	char buf[16]; // for integer to string conversion
	const char *p, *q;
	va_list ap;
	va_start(ap, fmt);
	for (q = p = fmt; *p; ++p) {
		if (*p == '%') {
			if (p > q) str_copy(s, q, p);
			++p;
			if (*p == 'd') {
				int c, i, l = 0;
				unsigned int x;
				c = va_arg(ap, int);
				x = c >= 0? c : -c;
				do { buf[l++] = x%10 + '0'; x /= 10; } while (x > 0);
				if (c < 0) buf[l++] = '-';
				str_enlarge(s, l);
				for (i = l - 1; i >= 0; --i) s->s[s->l++] = buf[i];
			} else if (*p == 'u') {
				int i, l = 0;
				uint32_t x;
				x = va_arg(ap, uint32_t);
				do { buf[l++] = x%10 + '0'; x /= 10; } while (x > 0);
				str_enlarge(s, l);
				for (i = l - 1; i >= 0; --i) s->s[s->l++] = buf[i];
			} else if (*p == 's') {
				char *r = va_arg(ap, char*);
				str_copy(s, r, r + strlen(r));
			} else if (*p == 'c') {
				str_enlarge(s, 1);
				s->s[s->l++] = va_arg(ap, int);
			} else abort();
			q = p + 1;
		}
	}
	if (p > q) str_copy(s, q, p);
	va_end(ap);
	s->s[s->l] = 0;
}

static inline char _hc_decode(uint8_t bc)
{
	return "acgtn"[bc];
}

static inline char _hc_encode(char c)
{
	extern unsigned char seq_nt4_table[256];
	return seq_nt4_table[(uint8_t) c];
}

static inline char _hc_rc(char c)
{
	return c >= 4 ? 4 : 3 - c;
}

static inline void _hc_encode_str(uint8_t *dst, const char *src, int len, int rc)
{
	if (rc) { // reverse_complement
		dst = dst + len - 1;
		for (int i=0; i<len; ++i)
			*dst-- = _hc_rc(_hc_encode(*src++));
	} else {
		for (int i=0; i<len; ++i)
			*dst++ = _hc_encode(*src++);
	}
}

static inline void _hc_align_append_skipped(kstring_t *s, uint8_t t, uint8_t q)
{
	hc_sprintf(s, "*%c%c", _hc_decode(t), _hc_decode(q));
}

static inline void _hc_align_append_mismatch(kstring_t *s, uint8_t t, uint8_t q)
{
	hc_sprintf(s, "*%c%c", _hc_decode(t), _hc_decode(q));
}

static inline void _hc_align_append_matches(kstring_t *s, int n)
{
	if (n) hc_sprintf(s, ":%d", n);
}

static inline int _hc_align_match_or_mismatch(kstring_t *s, int len, uint8_t **tseq, uint8_t **qseq)
{
	int ld = 0;
	int accumulated_matches = 0;
	for (int i=0; i < len; ++i) {
		if (**qseq == **tseq) {
			++accumulated_matches;
		} else {
			_hc_align_append_matches(s, accumulated_matches);
			_hc_align_append_mismatch(s, **tseq, **qseq);
			accumulated_matches = 0;
			ld += 1;
		};
		*tseq += 1;
		*qseq += 1;;
	}
	_hc_align_append_matches(s, accumulated_matches);
	return ld;
}

static inline int _hc_align_ref_insertion(kstring_t* s, int len, uint8_t **tseq, uint8_t **qseq)
{
	hc_sprintf(s, "+");
	for (int i=0; i < len; ++i) {
		hc_sprintf(s, "%c", _hc_decode(**qseq));
		*qseq += 1;
	}
	return len;
}

static inline int _hc_align_ref_deletion(kstring_t* s, int len, uint8_t **tseq, uint8_t **qseq)
{
	hc_sprintf(s, "-");
	for (int i=0; i < len; ++i) {
		hc_sprintf(s, "%c", _hc_decode(**tseq));
		*tseq += 1;
	}
	return len;
}

static inline int _hc_align_intron(kstring_t* s, int len, uint8_t **tseq, uint8_t **qseq)
{
	assert(len >= 2);
	uint8_t *tt = *tseq;
	hc_sprintf(s, "~%c%c%d%c%c", _hc_decode(tt[0]), _hc_decode(tt[1]),
		   _hc_decode(tt[len-2]), _hc_decode(tt[len-1]));
	*tseq += len;
	return len;
}


/// Align the query and the template sequences.
/// Returns the Levenshtein distance and the short cs tag in s
int hc_align_encoded(kstring_t *s, uint8_t *tseq, uint8_t *qseq, const mm_extra_t *x)
{
	int ld = 0; // Levenshtein distance
	for (int i = 0; i < x->n_cigar; ++i) {
		int op = x->cigar[i] & 0xf;
		int len = x->cigar[i] >> 4;
		assert(op >= 0 && op <= 3);
		if (op == 0) { // match or mismatch
			ld += _hc_align_match_or_mismatch(s, len, &tseq, &qseq);
		} else if (op == 1) { // insertion to ref
			ld += _hc_align_ref_insertion(s, len, &tseq, &qseq);
		} else if (op == 2) { // deletion from ref
			ld += _hc_align_ref_deletion(s, len, &tseq, &qseq);
		} else { // intron
			ld += _hc_align_intron(s, len, &tseq, &qseq);
		}
	}
	return ld;
}


hc_alignment_t hc_align(const char *seqstr, size_t seqlen, mm_idx_t* idx,
			const mm_reg1_t *r, hc_aln_ctx_t* aln_ctx)
{
	assert(r->p);
	char *tseq = cln_ctx->qseq;
	char *qseq = cln_ctx->qseq;
	mm_idx_getseq(idx, r->rid, r->rs, r->re, tseq); 	// get encoded ref
	_hc_encode_str(qseq, seqstr + r->qs, r->qe - r->qs, 0); // get encoded query
	hc_alignment_t details;
	details.qs = NULL;
	details.qe = NULL;
	if (r->qs > 0) {
		details.qs = (kstring_t *) calloc(1, sizeof(kstring_t));
		str_copy(details.qs, seqstr + r->qs, seqstr + r->qe);
	}
	if (r->qe < seqlen) {
		details.qe = (kstring_t *) calloc(1, sizeof(kstring_t));
		str_copy(details.qe, seqstr + r->qe, seqstr + seqlen);
	}
	details.alstr = (kstring_t *) malloc(sizeof(kstring_t));
	details.alstr->s = (char *) malloc(1025);
	details.alstr->l = 0;
	details.alstr->m = 1024;
	details.ldistance = hc_align_encoded(details.alstr, tseq, qseq, r->p);
	return details;
}



