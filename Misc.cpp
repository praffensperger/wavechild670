#include "Misc.h"

void do_assert_failed(const char *file, int line){
	fprintf(stderr, "Failure at %s : %i\n", file, line);
	throw 3;
}

