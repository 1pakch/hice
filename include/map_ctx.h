#pragma once

#include <minimap.h>

#include "hice/mem.h"


//! Reference index, mapping options and, possibly, thread-local storage
typedef struct {
	mm_idx_t *idx; 
	mm_idxopt_t *idxopt;
	mm_mapopt_t *mapopt;
	mm_tbuf_t *tbuf;
	size_t max_read_length;
} hc_map_ctx_t;


//! Reads or constructs an index and sets minimap2 mapping options
hc_map_ctx_t* hc_map_ctx_init_global(const char* reference_path,
				     size_t max_read_length,
				     size_t n_indexing_threads,
				     int verbose);


//! Creates a thread-local mapping context that uses thread-local memory
hc_map_ctx_t* hc_map_ctx_init_thread(const hc_map_ctx_t* global);


//! Frees either global or local mapping context
void hc_map_ctx_free(hc_map_ctx_t* map_ctx);
