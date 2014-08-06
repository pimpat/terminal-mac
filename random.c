#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

int main(int argc, char *argv[])
{
	char x[10]="";
	char ch; int r;     
//A=65	a=97
//Z=90	z=122
srand(time(NULL));
      for (int i = 0; i < 10; i++){
		ch = rand() % 26 + 65;         
		append(x,ch);
     	for (int j = 0; j < 4; j++){
		ch = rand() % 26 + 97;         
		//ch = r;
		//printf ("Random string %d: %c\n", r, ch);
		append(x,ch);
      	}
	printf("%s\n",x);
	x[0] = '\0';	
      }
}
