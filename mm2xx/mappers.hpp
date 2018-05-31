#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include <minimap.h>
#include <mm2xx/handles.hpp>
#include <mm2xx/mapped.hpp>
#include <mm2xx/settings.hpp>


namespace mm {


class MapperBase {
  private:
    const mm_idx_t *idx_;
    mm_mapopt_t mapopt_;
    handle<mm_tbuf_t> thread_buf_;

  protected:
    MapperBase(const Settings &settings, uint8_t n_max)
        : idx_(settings.idx())
        , mapopt_(*settings.mapopt())
        , thread_buf_(mm_tbuf_init()) {
        assert(idx_);
        assert(mapopt_.flag & MM_F_CIGAR);
        assert(thread_buf_.get());
        assert(n_max > 0);
        mapopt_.best_n = n_max;
    }

    const mm_mapopt_t *mapopt() const { return &mapopt_; }
    const mm_idx_t *idx() const { return idx_; }

    template<typename String> handle<mm_reg1_t[]> _map(String const &query) {
        int n_hits;
        mm_reg1_t *hits = mm_map(idx(), query.size(), query.data(), &n_hits,
                                 thread_buf_.get(), mapopt(), nullptr);
        assert(n_hits >= 0);
        handle<mm_reg1_t[]> result(hits, n_hits);
        return std::move(result);
    }
};


/// Mapper returning the best alignment for a given query
class Mapper : protected MapperBase {

  public:
    Mapper(const Settings &settings)
        : MapperBase(settings, 1) {}

    template<typename String> Mapped<> map(String const &query) {
        auto hits = _map(query);
        return std::move(Mapped<>(query, hits.get()));
    }
};

/// Mapper returning up to n_max alignments for a query
class MultiMapper : protected MapperBase {

  public:
    MultiMapper(const Settings &settings, uint8_t n_max)
        : MapperBase(settings, n_max) {}

    template<typename String> std::vector<Mapped<>> map(String const &query) {
        auto hits = _map(query);
        std::vector<Mapped<>> result;
        for (auto hit : hits) {
            result.emplace_back(query, &hit);
        }
        return std::move(result);
    }
};


} // namespace mm
