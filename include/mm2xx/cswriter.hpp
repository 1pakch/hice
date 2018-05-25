// cswriter.hpp - writing cs tag given cigar ops and query/reference strings

#pragma once

#include <include/mm2xx/encoding.hpp>


namespace mm {

class CsWriter {

	const enc::encoded *tseq;
	const enc::encoded *qseq;
	const mm_extra_t *x;

	size_t levenshtein = 0;
	size_t n_stored_matches = 0;
 	std::stringstream cs;

    public:

	CsWriter(const enc::encoded *tseq,
		 const enc::encoded *qseq,
		 const mm_extra_t *x)
	    : tseq(tseq)
	    , qseq(qseq)
	    , x(x)
	{}

	size_t align() {
		for (size_t i=0; i < x->n_cigar; ++i) {
			int op = x->cigar[i] & 0xf;
			size_t len = x->cigar[i] >> 4;
			assert(op >= 0 && op <= 3);
			switch (op) {
			    case 0: 
				process_match_or_mismatch(len);
				break;
			    case 1: 
				process_ref_insertion(len);
				break;
			    case 2: 
				process_ref_deletion(len);
				break;
			    case 3: 
				process_intron(len);
				break;
			}
		}
		return levenshtein;
	}

	std::unique_ptr<std::string> get_cs() const {
		return std::make_unique<std::string>(std::move(cs.str()));
	}

    private:

	inline void on_match() {
		n_stored_matches += 1;
	}

	inline void dump_matches() {
		if (n_stored_matches) {
			cs << ':' << n_stored_matches;
			n_stored_matches = 0;
		};
	}

	inline void on_mismatch() {
		cs << '*' << enc::decode(*tseq) << enc::decode(*qseq);
		levenshtein += 1;
	}

	inline void process_match_or_mismatch(size_t len) {
		for (size_t i=0; i < len; ++i) {
			if (*qseq == *tseq) {
				on_match();
			} else {
				dump_matches();
				on_mismatch();
			};
			++tseq;
			++qseq;
		}
		dump_matches();
	}

	inline void process_ref_insertion(size_t len) {
		cs << '+';
		for (size_t i=0; i < len; ++i) {
			cs << enc::decode(*qseq);
			++qseq;
		}
		levenshtein += len;
	}

	inline void process_ref_deletion(size_t len)
	{
		cs << '-';
		for (size_t i=0; i < len; ++i) {
			cs << enc::decode(*tseq);
			++tseq;;
		}
		levenshtein += len;
	}

	inline void process_intron(size_t len)
	{
		assert(len >= 2);
		cs << '~'
		  << enc::decode(tseq[0])
		  << enc::decode(tseq[1])
		  << len
		  << enc::decode(tseq[len-2])
		  << enc::decode(tseq[len-1]);
		tseq += len;
		levenshtein += len;
	}
};

} // mm
