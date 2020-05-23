#include "get_next_line.h"

#define BS 256
typedef struct s_cache
{
	char *content;
	struct s_cache *next;
} t_cache;

int hasnl(char *s)
{
	while (*s)
	{
		if (*s == '\n')
			return (1);
		s++;
	}
	return (0);
}

int lst_hasnl(t_cache *cache)
{
	while (cache)
	{
		if (hasnl(cache->content))
			return (1);
		cache = cache->next;
	}
	return (0);
}

t_cache *lst_new(char *content)
{
	t_cache *new;

	new = malloc(sizeof(t_cache));
	if (!new)
		return (0);
	new->content = content;
	new->next = 0;
	return (new);
}

t_cache *lst_add(t_cache **cache, char *content)
{
    t_cache *tmp = *cache;

    if (!content)
        return (0);
    if (!tmp)
        return (*cache = lst_new(content));
    else
    {
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = lst_new(content);
        return (tmp->next);
    }
}

char *ft_strdup(char *str, int len)
{
	char *res = malloc(len + 1);
	int i;

	if (!res)
		return (0);
	i = 0;
	while (*str)
		res[i++] = *str++;
	res[i] = 0;
	return (res);
}

int free_cache(t_cache **cache, int ret)
{
	t_cache *tmp;
	t_cache *nxt;

	if (*cache)
	{
		tmp = *cache;
		while (tmp)
		{
			nxt = tmp->next;
			free(tmp->content);
			free(tmp);
			tmp = nxt;
		}
		*cache = 0;
	}
	return (ret);
}

int lst_length(t_cache *cache)
{
	int total = 0;
	int i;

	while (cache)
	{
		i = 0;
		while (cache->content[i] && cache->content[i] != '\n')
			i++;
		total += i;
		if (cache->content[i] == '\n')
			return (total);
		cache = cache->next;
	}
	return (total);
}

int extract(t_cache **cache, char **line)
{
	int len = lst_length(*cache);
	int i, j;
	t_cache *nxt;

	*line = malloc(len + 1);
	if (!*line)
		return (free_cache(cache, -1));
	(*line)[len] = 0;
	i = 0;
	while (*cache)
	{
		j = 0;
		while ((*cache)->content[j] && (*cache)->content[j] != '\n')
			(*line)[i++] = (*cache)->content[j++];
		if ((*cache)->content[j++] == '\n')
		{
			i = 0;
			while ((*cache)->content[j])
				(*cache)->content[i++] = (*cache)->content[j++];
			(*cache)->content[i] = 0;
			return (1);
		}
		else
		{
			nxt = (*cache)->next;
			free((*cache)->content);
			free((*cache));
			*cache = nxt;
		}
	}
	return (0);
}

int get_next_line(char **line)
{
	static t_cache *cache = 0;
	char buff[BS + 1];
	int r_size;

	if (!lst_hasnl(cache))
	{
		while ((r_size = read(0, buff, BS)) > 0)
		{
			buff[r_size] = 0;
			if (!(lst_add(&cache, ft_strdup(buff, r_size))))
				return (free_cache(&cache, -1));
			if (hasnl(buff))
				break;
		}
	}
	return (extract(&cache, line));
}