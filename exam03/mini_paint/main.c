#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

typedef struct s_shape
{
	char type;
	float x;
	float y;
	float r;
	char color;
} t_shape;

typedef struct s_info
{
	int w;
	int h;
	char color;
} t_info;

int ft_strlen(char *s)
{
	int i = 0;
	while (s && s[i])
		i++;
	return (i);
}

int str_error(char *s)
{
	write(1, s, ft_strlen(s));
	return (1);
}

int clear_all(FILE *fp, char *canvas, char *err)
{
	if (fp)
		fclose(fp);
	if (canvas)
		free(canvas);
	if (err)
		str_error(err);
	return (1);
}

char *init_canvas(FILE *fp, t_info *info)
{
	char *canvas;
	int i;

	if (fscanf(fp, "%d %d %c\n", &info->w, &info->h, &info->color) != 3)
		return (0);
	if (info->w <= 0 || info->w > 300)
		return (0);
	if (info->h <= 0 || info->h > 300)
		return (0);
	if (!(canvas = malloc(info->w * info->h)))
		return (0);
	i = 0;
	while (i < info->w * info->h)
		canvas[i++] = info->color;
	return (canvas);
}

int in_circle(float x, float y, t_shape *sh)
{
	float dist;

	dist = sqrtf(powf(x - sh->x, 2.) + powf(y - sh->y, 2.));
	if (dist > sh->r)
		return (0);
	if (sh->r - dist < 1.00000000)
		return (2); // 경계
	return (1); // 경계보다 안
}

void draw_shape(t_info *info, char *canvas, t_shape *sh)
{
	int y;
	int x;
	int is_in;

	y = 0;
	while (y < info->h)
	{
		x = 0;
		while (x < info->w)
		{
			is_in = in_circle((float)x, (float)y, sh);
			if ((sh->type == 'c' && is_in == 2) || (sh->type == 'C' && is_in))
				canvas[(y * info->w) + x] = sh->color;
			x++;
		}
		y++;
	}
}

int draw_shapes(FILE *fp, t_info *info, char *canvas)
{
	t_shape sh;
	int ret;

	while ((ret = fscanf(fp, "%c %f %f %f %c\n", &sh.type, &sh.x, &sh.y, &sh.r, &sh.color)) == 5)
	{
		if (sh.r <= 0.00000000)
			return (0);
		if (sh.type != 'c' && sh.type != 'C')
			return (0);
		draw_shape(info, canvas, &sh);
	}
	if (ret != -1)
		return (0);
	return (1);
}

void print_canvas(t_info *info, char *canvas)
{
	int y = 0;

	while (y < info->h)
	{
		write(1, canvas + (y * info->w), info->w);
		write(1, "\n", 1);
		y++;
	}
}

int main(int ac, char **av)
{
	FILE *fp;
	t_info info;
	char *canvas;

	if (ac != 2)
		return str_error("Error: argument\n");
	if (!(fp = fopen(av[1], "r")))
		return str_error("Error: Operation file corrupted\n");
	if (!(canvas = init_canvas(fp, &info)))
		return clear_all(fp, NULL, "Error: Operation file corrupted\n");
	if (!draw_shapes(fp, &info, canvas))
		return clear_all(fp, canvas, "Error: Operation file corrupted\n");
	print_canvas(&info, canvas);
	clear_all(fp, canvas, NULL);
	return (0);
}