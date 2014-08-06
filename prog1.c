#include <stdio.h>

int main(void){
	float v1,v2;
	char op;
	printf("type expression: ");
	scanf("%f %c %f",&v1,&op,&v2);

	switch(op){
	case '+':
		printf("%.2f\n",v1+v2);
	break;
	case '-':
		printf("%.2f\n",v1-v2);
	break;
	case '*':
		printf("%.2f\n",v1*v2);
	break;
	case '/':
		if(v2==0) printf("division by 0.\n");
		printf("%.2f\n",v1/v2);
	break;
	default:		
	printf("unknown.\n");
	break;
	}
/*
	int u,v,temp;
	printf("enter 1st number: ");
	scanf("%i",&u);
	//printf("enter 2nd number: ");
	//scanf("%i",&v);
	
	do{
		temp = u%10;
		u/=10;	
		printf("%i",temp);		
	}
	while(u!=0);
	printf("\n");
//---------------------------------------------
	while(v!=0){
		temp = u%v;
		u=v;
		v=temp;
	}
	printf("gcd is %i.\n",u);
//---------------------------------------------
	int x = 25/7*7+25%7;
	int num;
	for(int i=0;i<3;i++){
		printf("insert number: ");
		scanf("%i",&num);
		//printf("%i\n",x);
		printf("%2i\n",num);
	}
*/
	return 0;
}
