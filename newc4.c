#include <stdio.h>
#include <stdlib.h>

struct cal{
    int a;
    int b;
    int (*add) (struct cal*);
    int (*subtract) (struct cal*);
    int (*multiply) (struct cal*);
};

int Add(struct cal* ar){
    return (ar->a + ar->b);
}

int Subtract(struct cal* ar){
    return (ar->a - ar->b);
}

int Multiply(struct cal* ar){
    return (ar->a * ar->b);
}

struct parent {
    int foo;
};

struct child {
    struct parent base;
    int bar;
};

struct person{
    	char name[10];
	int age;
    	char sex;
};

struct employee{
    struct person base2;
    char job[10];
};

int main(){
/*
    struct cal *ar=malloc(sizeof(struct cal));
    ar->a = 4;
    ar->b = 5;

    ar->add = Add;
    ar->subtract = Subtract;
    ar->multiply = Multiply;

    int a = ar->add(ar);
    int b = ar->subtract(ar);
    int c = ar->multiply(ar);    

    printf("%d\n%d\n%d\n", a, b, c);

	struct child *x = malloc(sizeof(struct child));
	x->bar = 1;
	x->base.foo = 2; 
    printf("%d\n%d\n", x->bar, x->base.foo);
*/
	struct employee *e1 = malloc(sizeof(struct employee));
	e1->base2.age=20;
    printf("%d\n", e1->base2.age);
    return 0;
}
