#include <stdio.h>

int main() {

	int c;

	while((c=getchar())!=EOF) {
		if(c==0x12) c='\'';
		putchar(c);
	}

	return 0;
}