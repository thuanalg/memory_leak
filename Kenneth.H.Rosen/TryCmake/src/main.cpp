#include <stdint.h>
#include <stdio.h>
#if INTPTR_MAX == INT32_MAX
	#define THIS_IS_32_BIT_ENVIRONMENT
	#define SPLNP		unsigned int
#elif INTPTR_MAX == INT64_MAX
	#define THIS_IS_64_BIT_ENVIRONMENT
	#define SPLNP		unsigned long long
#else
	#error "Environment not 32 or 64-bit."
#endif
int main(int argc, char *argv[]) {
	return 0;
}
//https://www.jetbrains.com/help/clion/cmakelists-txt-file.html#root-sub