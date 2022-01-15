#include <stdio.h>
#include "../output/params.h"

int main(void) {
    printf("%s\n", param_name(RATE_A));

    param_setu(RATE_A, 1337);
    printf("%d\n", param_getu(RATE_A));
    param_dump("params.dat");
    param_setu(RATE_A, 42);
    param_load("params.dat");
    printf("%d\n", param_getu(RATE_A));
}
