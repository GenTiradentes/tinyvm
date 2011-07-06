#include "tvm_memory.h"

#include <string.h>

memory* create_memory(size_t size)
{
	memory* mem;

	int num_words = size / sizeof(int);

	mem = (memory*)malloc(sizeof(memory));
	mem->int32 = (int*)malloc(num_words * sizeof(int));
	mem->max = 0;

	mem->FLAGS = 0;
	mem->remainder = 0;

	return mem;
}

void destroy_memory(memory* mem)
{
	free(mem->int32);
	free(mem);
}
