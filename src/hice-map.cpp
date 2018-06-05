#include <thread>
#include <utility>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cassert>

#include <cxxopts.hpp>
#include <bpcqueue.hpp>

#include <mm2xx/mappers.hpp>
#include <hice/align.hpp>
#include <hice/pair.hpp>
#include <hice/fastx.hpp>
#include <hice/gzstream.hpp>
#include <hice/tagged.hpp>


using namespace mm;
using namespace hc;
using namespace bpcqueue;


using Read = std::string;
using Alignment = Aligned<>;
using Path = const char*;



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
    size_t n = 0;
    while (alignments.pop(al)) {
        if (al)
            printf("R%1zd>  %3zd  %c  %9zd:+%-3zd  %2zd>%3zd<%-2zd  q=%-2zd  d=%-3zd  cs=%s;  qs=%s;  qe=%s\n",
                    n % 2 + 1,
		    al.rid1(),
                    "+-"[al.is_rc()],
                    al.rstart(), al.rlen(), al.qstart(), al.qlen(), al.qe_str().size(),
                    al.mapq(), al.ldist(),
                    al.cs().c_str(), al.qs_str().c_str(), al.qe_str().c_str());
        else
            printf("R%1zd>    0\n", n % 2 + 1);
	++n;
    }
    fprintf(stderr, "Processed %zd reads (%zd pairs)\n", n, n/2);
}

int process(const Settings& settings,
            Tagged<Path, fastx::Format> reads1_path,
            Tagged<Path, fastx::Format> reads2_path,
            size_t n_threads) 
{
	Queue<Read> reads1(1024), reads2(1024);
        Queue<Pair<Read>> paired_reads(1024);
        Queue<Pair<Alignment>> paired_als(1024);
        Queue<Alignment> als(1024);

        std::thread parser1, parser2;
        std::vector<std::thread> mappers;

        parser1 = std::thread(fastx::parse, reads1_path, output(reads1), 10);
	parser2 = std::thread(fastx::parse, reads2_path, output(reads2), 10);

        auto zipper = std::thread(pair<Read>, input(reads1), input(reads2), output(paired_reads));
        for (size_t i=0; i < n_threads; ++i)
            mappers.emplace_back(align_paired, input(paired_reads), output(paired_als), std::ref(settings));

        auto unzipper = std::thread(unpair<Alignment>, input(paired_als), output(als));

	auto printer = std::thread(print, input(als));
	
	parser1.join();
	parser2.join();
	zipper.join();

        for (size_t i=0; i < n_threads; ++i)
	    mappers[i].join();

	unzipper.join();
	printer.join();

        return 0;
}


int main(int argc, char *argv[])
{

    cxxopts::Options options("hice-map", "Aligner for Hi-C data based on minimap2");
    options
        .positional_help("REFERENCE FILE1 FILE2")
        .show_positional_help()
        .add_options()
            ("h,help", "Show this message and exit")
            ("n,threads", "Number of worker threads", cxxopts::value<size_t>()->default_value("1"));

    options
        .add_options("hidden")
            ("files", "paths of FASTA/FASTQ files with reads", cxxopts::value<std::vector<std::string>>())
        ;
    options.parse_positional({"files"});
    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help({""}) << std::endl;
        return 0;
    }
   
    const size_t n_threads = args["threads"].as<size_t>();
    assert(n_threads > 0 && n_threads < 32);

    auto files = args["files"].as<std::vector<std::string>>();
    assert(files.size() == 3);
    auto reads1_tagged_path = tagged((Path) files[1].c_str(), fastx::guess_format(files[1].c_str()));
    auto reads2_tagged_path = tagged((Path) files[2].c_str(), fastx::guess_format(files[2].c_str()));
    assert(reads1_tagged_path.tag != fastx::Format::ambiguous);
    assert(reads2_tagged_path.tag != fastx::Format::ambiguous);
    
    Settings settings("sr");
    settings.index_file(files[0].c_str(), 10);

    std::cerr << "Starting main loop" << '\n';
    process(settings,
            reads1_tagged_path,
            reads2_tagged_path,
            n_threads);
    

    fflush(stdout);
    return 0;
}
