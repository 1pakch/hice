#include <thread>
#include <utility>
#include <stdlib.h>
#include <stdio.h>

#include <bpcqueue.hpp>

#include <mm2xx/mappers.hpp>
#include <hice/align.hpp>
#include <hice/pair.hpp>
#include <hice/gzstream.hpp>


using namespace mm;
using namespace hc;
using namespace bpcqueue;


using Read = std::string;
using Alignment = Aligned<>;


void align_paired(Input<Pair<Read>> paired_reads,
         Output<Pair<Alignment>> aligned_pairs,
         const Settings& settings)
{
    Aligner a(settings);
    Pair<Read> paired_read;
    while (paired_reads.pop(paired_read)) {
        auto r1 = paired_read.get_first();
        auto r2 = paired_read.get_second();
        //printf("1: %s\n", r1.c_str());
        //printf("2: %s\n", r2.c_str());
        aligned_pairs.emplace(
            std::move(a.map(std::move(r1))),
            std::move(a.map(std::move(r2)))
        );
    }
}

void print(Input<Alignment> alignments) {
    Alignment al;
    while (alignments.pop(al)) {
        if (al)
            printf("%zd,\t%zd,\t%zd,\t%zd,\t%zd,\t%zd,\t%zd,\t%s,\t%s,\t%s\n",
                    al.rid1(), al.rstart(), al.rend(), al.mapq(),
                    al.qs(), al.qe(), al.ldist(),
                    al.cs()->c_str(), al.qs_str()->c_str(), al.qe_str()->c_str()
                        );
        else
            printf("Unaligned\n");
    }
}

int process(const Settings& settings, gzfile reads_file1, gzfile reads_file2) {
	Queue<std::string> chunks1(4), chunks2(4);
	Queue<Read> reads1(100), reads2(100);
        Queue<Pair<Read>> paired_reads(100);
        Queue<Pair<Alignment>> paired_als(100);
        Queue<Alignment> als(200);

        auto streamer1 = std::thread(gzstream, output(chunks1), std::move(reads_file1), 4096);
	auto parser1 = std::thread(split_lines<2, 1>, input(chunks1), output(reads1));

        auto streamer2 = std::thread(gzstream, output(chunks2), std::move(reads_file2), 4096);
	auto parser2 = std::thread(split_lines<2, 1>, input(chunks2), output(reads2));
        auto zipper = std::thread(pair<Read>, input(reads1), input(reads2), output(paired_reads));
        
        auto mapper = std::thread(align_paired, input(paired_reads), output(paired_als), std::ref(settings));
        auto unzipper = std::thread(unpair<Alignment>, input(paired_als), output(als));

	auto printer = std::thread(print, input(als));
	
	streamer1.join();
	streamer2.join();
	parser1.join();
	parser2.join();
	zipper.join();
	mapper.join();
	unzipper.join();
	printer.join();

        return 0;
}


int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "Usage: REFERENCE READS1 READS2\n");
		return -1;
	}

	Settings settings("sr");
	settings.index_file(argv[1]);
 
        process(settings,
                std::move(gzfile(argv[2], "r", 4096)),
                std::move(gzfile(argv[3], "r", 4096)));
        

	fflush(stdout);
        return 0;
}
