#include <thread>
#include <utility>
#include <stdlib.h>
#include <stdio.h>

#include <bpcqueue.hpp>

#include <mm2xx/mappers.hpp>
#include <hice/pair.hpp>
#include <hice/gzstream.hpp>


using namespace mm;
using namespace hc;
using namespace bpcqueue;


using Read = std::string;
using RawAlignments = handle<mm_reg1_t[]>;



void map(Input<Read> reads,
         Output<RawAlignments> alignments,
         const Settings& settings)
{
    MapperBase m(settings, 10);
    std::string seq; 
    while (reads.pop(seq)) {
        alignments.push(std::move(m.map(std::move(seq))));
    }
}

void map_paired(Input<Pair<Read>> paired_reads,
         Output<Pair<RawAlignments>> paired_alignments,
         const Settings& settings)
{
    MapperBase m(settings, 10);
    Pair<Read> paired_read;
    while (paired_reads.pop(paired_read)) {
        paired_alignments.emplace(
            std::move(m.map(paired_read.get_first())),
            std::move(m.map(paired_read.get_second()))
        );
    }
}

void print(Input<RawAlignments> alignments) {
    RawAlignments al;
    while (alignments.pop(al)) {
        if (al)
            printf("%d\t%d\t%d\t%d\n", al[0].rid, al[0].rs, al[0].re, al[0].mapq);
        else
            printf("Unaligned\n");
    }
}


int process(const Settings& settings, gzfile reads_file) {
	Queue<std::string> chunks(4);
	Queue<Read> reads(4);
	Queue<RawAlignments> alignments(4);

        auto streamer = std::thread(gzstream, output(chunks), std::move(reads_file), 4096);
	auto parser = std::thread(split_lines<2, 1>, input(chunks), output(reads));
	auto mapper = std::thread(map, input(reads), output(alignments), std::ref(settings));
	auto printer = std::thread(print, input(alignments));

	streamer.join();
	parser.join();
	mapper.join();
	printer.join();
	return 0;
}

int process(const Settings& settings, gzfile reads_file1, gzfile reads_file2) {
	Queue<std::string> chunks1(4), chunks2(4);
	Queue<Read> reads1(100), reads2(100);
        Queue<Pair<Read>> paired_reads(100);
        Queue<Pair<RawAlignments>> paired_als(100);
        Queue<RawAlignments> als(200);

        auto streamer1 = std::thread(gzstream, output(chunks1), std::move(reads_file1), 4096);
	auto parser1 = std::thread(split_lines<2, 1>, input(chunks1), output(reads1));

        auto streamer2 = std::thread(gzstream, output(chunks2), std::move(reads_file2), 4096);
	auto parser2 = std::thread(split_lines<2, 1>, input(chunks2), output(reads2));
        auto zipper = std::thread(pair<Read>, input(reads1), input(reads2), output(paired_reads));
        
        auto mapper = std::thread(map_paired, input(paired_reads), output(paired_als), std::ref(settings));
        auto unzipper = std::thread(unpair<RawAlignments>, input(paired_als), output(als));

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
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "Usage: REFERENCE FASTA1 [FASTA2]\n");
		return -1;
	}

	Settings settings("sr");
	settings.index_file(argv[1]);

        if (argc == 3)
            process(settings,
                    std::move(gzfile(argv[2], "r", 4096)));
        
        else if (argc == 4)
            process(settings,
                    std::move(gzfile(argv[2], "r", 4096)),
                    std::move(gzfile(argv[3], "r", 4096)));
        

	fflush(stdout);
        return 0;
}
