#include <sys/time.h>
#include <stdio.h>
void current_timestamp(){
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    //long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    //printf("milliseconds: %lld\n", milliseconds);
    printf("milliseconds: %ld\n", milliseconds);
    //return milliseconds;
}

int main(int argc, char* argv[]){
current_timestamp();	
	return 0;
}
