#include <unistd.h>

// print charactor exist both in s1, s2
// ex) s1 : "my name is", s2 : "woolim"
// res : "mi"

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

int ft_strlen(char *s)
{
    char *b;

    b = s;
    while (*s != '\0')
        s++;
    return (s - b);
}

void ft_inter(char *s1, char *s2)
{
    int i;
    int len;

    i = 0;
    len = ft_strlen(s2);
    while (s1[i] != '\0')
    {
        if (!ck_char(s1, s1[i], i) && ck_char (s2, s1[i], len))
            write(1, &s1[i], 1);
        i++;
    }
}

int main(int argc, char **argv)
{
    if (argc == 3)
        ft_inter(argv[1], argv[2]);
    write(1, "\n", 1);
    return (0);
}