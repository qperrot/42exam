#include <stdio.h>
int ft_printf(char *, ...);

int main(void)
{
    int i, j;

    i = printf("|%x, %d, %s|\n", -2147483647 * 4, 54546, "asdfasdfa");
    j = ft_printf("|%x, %d, %s|\n", -2147483647 * 4, 54546, "asdfasdfa");
    printf("real %d, mine %d\n", i, j);

    i = printf("|%3.5x, %3.5d, %s|\n", 0, 0, "asdfasdfa");
    j = ft_printf("|%3.5x, %3.5d, %s|\n", 0, 0, "asdfasdfa");
    printf("real %d, mine %d\n", i, j);

    i = printf("|%3.x, %3.5d, %s|\n", 0, 123, NULL);
    j = ft_printf("|%3.x, %3.5d, %s|\n", 0, 123, NULL);
    printf("real %d, mine %d\n", i, j);

    return (0);
}