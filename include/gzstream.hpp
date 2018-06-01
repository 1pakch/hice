#pragma once

#include <string>

#include <bpcqueue.hpp>

#include <hice/gzfile.hpp>


namespace hc {

void gzstream(bpcqueue::Output<std::string> dst, gzfile f, size_t chunk_size) {
	size_t bytes_read;
	do {
		dst.push(std::move(f.readstr(chunk_size, &bytes_read)));
	} while (bytes_read == chunk_size);
}


template<size_t take_every=1, size_t skip=0>
void split_lines(bpcqueue::Input<std::string> chunks,
                 bpcqueue::Output<std::string> lines) {
    std::string buf;
    std::string tail;
    size_t lineno = 0;
    size_t start = 0;
    auto take_this_line = [&](){
        return lineno >= skip && ((lineno - skip) % take_every == 0);
    };

    while (chunks.pop(buf)) {
        for (size_t i=0; i<buf.size(); ++i) {
            if (buf[i] == '\n') {
                if (take_this_line()) {
                    auto line = std::move(tail);
                    line.append(buf, start, i - start);
                    lines.push(std::move(line));
                }
                tail = std::string();
                ++lineno;
                start = i+1;
            }
        }
        tail = std::string(buf, start, buf.size());
    }
}

}
