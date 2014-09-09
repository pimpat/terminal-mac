#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void addnum(int *x);

typedef struct book {
	char name[10];
	int price;
} Book;

int main(){
/*
	int x[30];
	typedef struct{
		int *y;
	}mystruct;
	x[0]=1;
	x[20]=111;
	x[21]=222;
	x[25]=333;
	mystruct a;
	
	a.y=&x[20];
	//printf("%d\n",x);
	//printf("%d\n",&x[0]);
	printf("%d\n",*a.y);
	//printf("%d\n",a.y);
	printf("%d\n",*(a.y+1));
	printf("%d\n",*(a.y-20));
*/
/*
	char str[20];
	char x[10];
	char y[10];
	printf("text: ");
	scanf("%s",str);
	sscanf(str, "%[^'/']/4%s",x,y);	
	printf("%s\t%s\n",x,y);
	system("mkdir testcmd");

	FILE *fp;
	fp = fopen("/Users/pimpat/testcmd/tmptext.txt","a");
	//fputs("hi\nhello\thehe\n",fp);
	//fprintf(fp,"%s\n","12345");

	printf("end\n");
*/
	enum{pim,pat};
	printf("pim: %d\tpat: %d\n",pim,pat);
	int x=0;
	printf("x: %d\n",x);
	addnum(&x);
	printf("x: %d\n",x);

	Book b;
	Book *ptr = &b;
	b.price = 20;
	printf("b-ptr: %d\n",ptr->price);
	printf("b: %d\n",b.price);
	ptr->price = 30;
	printf("b-ptr: %d\n",ptr->price);
	printf("b: %d\n",b.price);

	Book a[10];
	Book *ptra = a;
	ptra[1].price = 10;
	ptra[2].price = 20;
	a[3].price = 30;
	printf("a[1]: %d\n",a[1].price);
	printf("a[2]: %d\n",a[2].price);
	printf("ptra[3]: %d\n",ptra[3].price);
	printf("addr p: %d\n",(int)ptra);
	printf("addr: %d\n",(int)a);

	return 0;
}
void addnum(int *x){
	*x = 5;
}
