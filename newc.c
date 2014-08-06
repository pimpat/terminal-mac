#include <stdio.h>
#include <stdlib.h>

//void doSth(int* const myPtr);
void doSth(int *myPtr,int x){
	*myPtr=30;
	printf("in function\np addr: %d\n",myPtr);
	myPtr=&x;
	printf("p addr: %d\n",myPtr);
}
int test(int x,int y){
	return x*y;
}
int (*myfunc)(int x, int y);

int main(){
	struct pointptr{
		int *x;
		int *y;
	};
	struct pointptr myPointptr;
	struct point{
        	int x;
		int y;
	};
	struct point myPoint;
/*
	struct point *createPoint(int x, int y){
		struct point *temp = malloc(sizeof(struct point));
		if(temp != NULL)
			return (struct point *) temp;
		else
			return NULL;
	};
*/
	myfunc = test;
	int result = myfunc(10,25);
	printf("result: %d\n",result);

	char myArray[20];
	char *myPt;
	myPt = myArray;
	*(myPt + 0) = 'A';
	*(myPt + 1) = 'A';
	*(myPt + 2) = 'A';
	*(myPt + 3) = '\0';
	printf("Text: %s\n",myArray);
/*
	int u = 12;
	int v = 13;
	int *p = &u;
	printf("p value: %d\n",*p);
	printf("p addr: %d\n",p);
	doSth(p,v);
	printf("p value: %d\n",*p);
	printf("p addr: %d\n",p);
*/
/*
	myPoint.x=10;
	myPoint.y=20;
	printf("%d\n",myPoint.x);
	printf("%d\n",myPoint.y);
	struct point *myPoint2 = malloc(sizeof(struct point));
	(*myPoint2).x=30;
	myPoint2->y=40;
	printf("%d\n",myPoint2->x);
	printf("%d\n",myPoint2->y);
	printf("%s\n","------------------");
	int x=15,y=25;
	myPointptr.x=&x;
	myPointptr.y=&y;
	printf("%d\n",*myPointptr.x);
	printf("%d\n",*myPointptr.y);
*/
	char c='x';
	char c2='y';
	char *c_ptr2=&c2;
	char *c_ptr=&c;
	char *const myConstptr=&c;
	printf("val const: %d\n", *myConstptr);
	printf("addr const: %d\n", myConstptr);
	char d='d';
	//myConstptr=&d;
	*myConstptr='A';
	printf("change value\n");
	printf("val const: %d\n", *myConstptr);
	printf("addr const: %d\n", myConstptr);
	const char *myOtherptr=&c;
	printf("\nval const: %d\n", *myOtherptr);
	printf("addr const: %d\n", myOtherptr);
	//*myOtherptr='q';
	myOtherptr=&d;	
	printf("change address\n");
	printf("val const: %d\n", *myOtherptr);
	printf("addr const: %d\n", myOtherptr);
	
	const char* const myPtr=&c;
	//*myPtr='a';
	//myPtr=&d;	

	int i,j;
	int *ptr1, *ptr2;
	i=1;
	j=2;
	ptr1=&i;
	ptr2=&j;
	printf("val p1: %d\n", *ptr1);
	printf("val p2: %d\n", *ptr2);
	printf("addr p2: %d\n", ptr2);
	ptr1=ptr2;
	printf("val p1: %d\n", *ptr1);
	printf("addr p1: %d\n", ptr1);

	return 0;
}

