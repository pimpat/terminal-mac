#include <stdio.h>
#include <uuid/uuid.h>

int main(){
	uuid_t uuid;
	uuid_generate(uuid);
	uuid_string_t x;
	uuid_unparse(uuid,x);
	printf("%s\n",x);
	return 0;
}
