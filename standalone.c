#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const * argv[])
{
	if (argc != 2) {
		printf("usage: %s <file>\n", argv[0]);
		return EXIT_FAILURE;
	}
	FILE * file = fopen(argv[1], "r");
	if (file == NULL) {
		printf("can't open file '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}
	fclose(file);
	return EXIT_SUCCESS;
}

