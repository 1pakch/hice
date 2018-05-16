/// Constructs and destroys a mapping index and associated data structures

#include "hice/map_ctx.h"


int main(int argc, char *argv[])
{
	static const char* refpath = "testdata/ref.fa";
	hc_map_ctx_t* map_ctx = hc_map_ctx_init_global(refpath, 0, 4, 10);
	hc_map_ctx_free(map_ctx);
}
