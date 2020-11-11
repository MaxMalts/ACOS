#include <stdio.h>

int main() {
	char str[100] = "";
	scanf("%[^.]s", str);
	printf("prog1got'%s'", str);
	fflush(stdout);

	return 0;
}