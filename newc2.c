#include <stdio.h>
#include <stdlib.h>
int main(){
	int x[4];
	int *i1 = (int*) malloc(sizeof(int)*5);	// garbage value (risk)
	int *i2 = (int*) calloc(5,sizeof(int));	// initialize to 0
	long y[4];
	long *l = (long*) malloc(sizeof(long));
	char z[4];
	char *c = (char*) malloc(sizeof(char));
	printf("---------------------\n");
	printf("int array: %lu\n",sizeof(x));
	printf("long array: %lu\n",sizeof(y));
	printf("char array: %lu\n",sizeof(z));
	printf("size pointer int: %lu\n",sizeof(*i1));
	printf("size pointer long: %lu\n",sizeof(*l));
	printf("size pointer char: %lu\n",sizeof(*c));

	for(int i=0;i<5;i++)
		printf("a: %d\tb: %d\n",i1[i],i2[i]);
	// values are zero both calloc and malloc.

return 0;
}
