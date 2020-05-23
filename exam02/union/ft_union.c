#include <unistd.h>

// print every charactor without double
// ex) "hi, my name is" "woolim"
// res : "hi, mynaeswol"

int ck_char(char *s, char c, int idx)
{
    int i;

    i = 0;
    while (i < idx)
    {
        if (s[i] == c)
            return (1);
        i++;
    }
    return (0);
}

void ft_union(char *s1, char *s2)
{
    int i;
    int len;

    i = 0;
    while (s1[i])
    {
        if (!ck_char(s1, s1[i], i))
            write(1, &s1[i], 1);
        i++;
    }
    len = i;
    i = 0;
    while (s2[i])
    {
        if (!ck_char(s1, s2[i], len) && !ck_char(s2, s2[i], i))
            write(1, &s2[i], 1);
        i++;
    }
}

int main(int argc, char **argv)
{
    if (argc == 3)
        ft_union(argv[1], argv[2]);
    write(1, "\n", 1);
    return (0);
}