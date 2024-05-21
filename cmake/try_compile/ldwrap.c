#include <stdlib.h>

void __real_exit(int status);

void
__wrap_exit(int s)
{
    __real_exit(0);
}

int
main()
{
    exit(1);
    return 0;
}
