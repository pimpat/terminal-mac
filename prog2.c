#include <stdio.h>
#include <stdbool.h>
int main(){
	int n;
	printf("enter number: ");
	scanf("%i",&n);
	int p,i,prime[n],index=2;
	bool isPrime;
	prime[0]=2;
	prime[1]=3;
	for(p=5;p<=n;p=p+2){
		isPrime=true;
		for(i=1;isPrime&&p/prime[i]>=prime[i];i++)
			if(p%prime[i]==0) isPrime=false;
		if(isPrime==true){
			prime[index]=p;
			index++;
		}
	}	
	for(i=0;i<index;i++) printf("%i\t",prime[i]);
	printf("\n");

/*
	int n;
	printf("enter number: ");
	scanf("%i",&n);
	int fi[n+1];
	fi[0]=0;
	fi[1]=1;
	for(int i=2;i<=n;i++) fi[i]=fi[i-2]+fi[i-1];
	printf("answer is %i.\n",fi[n]);
//---------------------------------------------
	int rate[11],res;
	for(int i=1;i<=10;i++) rate[i]=0;
	for(int i=1;i<=10;i++){
		printf("enter points[%i]: ",i);
		scanf("%i",&res);
		if(res<1||res>10) printf("bad res: %i\n",res);
		else rate[res]++;
	}
	for(int i=1;i<=10;i++) printf("%4i%4i\n",i,rate[i]);
*/
return 0;
}
