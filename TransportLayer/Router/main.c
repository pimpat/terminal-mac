#include <stdio.h>
#include "router.h"

int main(int argc, const char * argv[])
{
    startRouter();
    while (1);
    stopRouter();
    
    return 0;
}