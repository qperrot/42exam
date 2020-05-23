#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "get_next_line.h"

int main(void)
{
    int r;
    char *line;

    line = NULL;
    int i = 0;
    while ((r = get_next_line(&line)) > 0)
    {
        printf("%d |%s|\n", r, line);
        free(line);
        line = NULL;
    }
    printf("%d |%s|\n", r, line);
    free(line);
    line = NULL;
    return (0);
}