// cswriter.hpp - writing cs tag given cigar ops and query/reference strings

#pragma once

#include <mm2xx/encoding.hpp>


namespace mm {

class CsStringWriter {
  private:
    std::stringstream cs_;
    size_t pos_ = 0;
    bool perfect_match_ = true;

    void imperfect_match() {
        if (perfect_match_) {
            perfect_match_ = false;
            if (pos_) append_match(pos_);
        }
    }

    inline void append_match(size_t len){
        cs_ << ':' << len;
    }

  public:

    CsStringWriter() {}

    std::string result() const { return std::move(cs_.str()); }

    //! match of length len
    inline void on_match(size_t len) {
        if (!perfect_match_) append_match(len);
        pos_ += len;
    }

    //! single mismatch
    inline void on_mismatch(enc::encoded in_ref, enc::encoded in_query) {
        imperfect_match();
        cs_ << '*' << enc::decode(in_ref) << enc::decode(in_query);
    }

    //! insertion into reference of length len
    inline void on_insertion(const enc::encoded *seq, size_t len) {
        imperfect_match();
        cs_ << '+';
        for (size_t i = 0; i < len; ++i)
            cs_ << enc::decode(seq[i]);
    }

    //! deletion from reference of length len
    inline void on_deletion(const enc::encoded *seq, size_t len) {
        imperfect_match();
        cs_ << '-' << len;
    }

    //! intron of length len
    inline void on_intron(const enc::encoded *seq, size_t len) {
        imperfect_match();
        cs_ << 'i' << len;
    }

};


class CigarIterator {

    const enc::encoded *tseq;
    const enc::encoded *qseq;
    const mm_extra_t *x;

    size_t ldist_ = 0;  // Levenshtein distance
    size_t qlen_ = 0;

  public:
    CigarIterator(const enc::encoded *tseq, const enc::encoded *qseq,
             const mm_extra_t *x)
        : tseq(tseq)
        , qseq(qseq)
        , x(x) {}

    template<class F>
    auto map(F& f) {
        int opcode;
        size_t len;
        auto qstart = qseq;
        for (size_t i = 0; i < x->n_cigar; ++i) {
            parse_cigar(x->cigar[i], opcode, len);
            switch (opcode) {
                case 0:
                    process_match_or_mismatch<F>(f, len);
                    break;
                case 1:
                    f.on_insertion(qseq, len);
                    qseq += len;
                    ldist_ += len;
                    break;
                case 2:
                    f.on_deletion(tseq, len);
                    tseq += len;
                    ldist_ += len;
                    break;
                case 3:
                    assert(len >= 2);
                    f.on_intron(tseq, len);
                    tseq += len;
                    ldist_ += len;
                    break;
            }
        }
        qlen_ = qseq - qstart;
    }

    size_t ldist() const { return ldist_; }
    size_t qlen() const { return qlen_; }

  private:

    void parse_cigar(int cigar, int& opcode, size_t& len) {
        opcode = cigar & 0xf;
        len = cigar >> 4;
        assert(opcode >= 0 && opcode <= 3);
    }

    template<class F>
    inline void process_match_or_mismatch(F& f, size_t len) {
        size_t n_matches = 0;
        for (size_t i = 0; i < len; ++i) {
            if (*qseq == *tseq) {
                n_matches += 1;
            } else {
                if (n_matches)
                    f.on_match(n_matches);
                n_matches = 0;
                f.on_mismatch(*qseq, *tseq);
                ldist_ += 1;
            };
            ++tseq;
            ++qseq;
        }
        if (n_matches)
            f.on_match(n_matches);
    }

};

} // namespace mm
