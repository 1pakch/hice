
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <minimap.h>
#include <kseq.h>
#include <pipe.h>

#include <hc_wthread.h>
#include <hc_rthread.h>
#include <hc_athread.h>

#define NWORKERS 2


void print_usage()
{
	fprintf(stderr, "Usage: hicmap <target.fa> <R1.fa> <R2.fa>\n");
}

int main(int argc, char *argv[])
{
	char index_fname[] = "testdata/ref.fa";
	char *read_fnames[2] = { "testdata/r1.fa", "testdata/r2.fa" };
	char **reads = read_fnames;
	char *index = index_fname;

	if (argc == 1) {
		//print_usage();
		//return 1;
	} else {
		index = argv[1];
		reads = &argv[2];
	}

	// read one part of the index (multi-part indexes are not supported)
	mm_idxopt_t iopt;
	mm_mapopt_t mopt;
	int n_threads = 3;
	mm_verbose = 2; // disable message output to stderr
	mm_set_opt(0, &iopt, &mopt);
	mopt.flag |= MM_F_CIGAR; // Perform alignment
	mopt.flag |= MM_F_SR;    // Short reads profile
	// mopt.end_bonus = 20;
	mm_idx_t *mi = mm_idx_read(index, &iopt, n_threads);
	if (!mi) return 2;
	mm_mapopt_update(&mopt, mi); // this sets the maximum minimizer occurrence; 

	// Create pipes
	pipe_t* raw = pipe_new(sizeof(hc_read_pair_t), 2000);
	//pipe_reserve(PIPE_GENERIC(raw), 2000);
	pipe_t* mapped = pipe_new(sizeof(hc_mapped_pair_t), 2000);
	//pipe_reserve(PIPE_GENERIC(mapped), 2000);
	
	// Start a thread reading raw pairs from argv[2] and argv[3]
	hc_rthread_t* rthread  = hc_rthread_create(reads, raw, 1000);
	if (!rthread) return 3;

	// Start aligner threads
	hc_athread_t* athreads[2];
	for (int i=0; i<NWORKERS; ++i) {
		athreads[i] = hc_athread_create(mi, &mopt, raw, mapped, 100);
	}
	
	// The pipe from raw pairs to aligners is established
	pipe_free(raw);
	
	// Start a writing (output) thread
	hc_wthread_t* wthread = hc_wthread_create(mapped, 1000);
	pipe_free(mapped);

	// Wait for the reading thread to finish
	hc_rthread_join(rthread);

	// Wait for the alighers to finish
	for (int i=0; i<NWORKERS; ++i) {
		hc_athread_join(athreads[i]);
	}

	// Wait for the writer (output) thread to finish
	hc_wthread_join(wthread);

	mm_idx_destroy(mi);
	return 0;
}
