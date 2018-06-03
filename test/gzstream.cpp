#include <string>
#include <thread>
#include <unistd.h>

#include <bpcqueue.hpp>
#include <hice/gzstream.hpp>


using namespace hc;
using namespace bpcqueue;

static constexpr bool verbose = false;


void print(bpcqueue::Input<std::string> src) {
    if (verbose) printf("C: mainloop\n");
    std::string s;
    while (src.pop(s)) {
        if (verbose)
            printf("C: received '%s'\n", s.c_str());
        else
            printf("%s\n", s.c_str());
    }
    if (verbose) printf("C: ended\n");
}

int main(int argc, const char **argv) {
    if (argc != 2) {
            printf("Usage: %s GZIPPED_OR_PLAIN_TEXT_FILE\n", argv[0]);
            return -1;
    }

    if (verbose) printf("M: Creating queues\n");
    Queue<std::string> chunks(4);
    Queue<std::string> lines(4);

    if (verbose) printf("M: Starting streamer\n");
    auto r = std::thread(gzstream, output(chunks), gzfile(argv[1], "r", 10), 4096);
    if (verbose) printf("M: Starting splitter\n");
    auto s = std::thread(split_lines<2, 1>, input(chunks), output(lines));
    if (verbose) printf("M: Starting printer\n");
    auto p = std::thread(print, input(lines));

    r.join();
    if (verbose) printf("M: streamer joined\n");
    s.join();
    if (verbose) printf("M: splitter joined\n");
    p.join();
    if (verbose) printf("M: printer joined\n");
    
    fflush(stdout);
    return 0;
}
