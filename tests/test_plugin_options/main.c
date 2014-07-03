#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "plugin_options.h"

extern void some_test();

int main(int argc, char **argv)
{
    some_test();
    return 0;
}