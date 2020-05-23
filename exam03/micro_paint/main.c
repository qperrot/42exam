#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct	s_info
{
	int		w;
	int		h;
	char	color;
} 				t_info;

typedef struct	s_shape
{
	char	type;
	float	x;
	float	y;
	float	w;
	float	h;
	char	color;
}				t_shape;

int
	ft_strlen(char const *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

int str_error(char const *str)
{
	write(1, str, ft_strlen(str));
	return (1);
}

int	clear_all(FILE *file, char *canvas, char *err)
{
	if (file)
		fclose(file);
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

int in_rect(float x, float y, t_shape *sh)
{
	if (x < sh->x || x > sh->x + sh->w)
		return (0); // 바깥
	if (y < sh->y || y > sh->y + sh->h)
		return (0); // 바깥
	if (x - sh->x < 1.00000000 || sh->x + sh->w - x < 1.00000000)
		return (2); // 경계
	if (y - sh->y < 1.00000000 || sh->y + sh->h - y < 1.00000000)
		return (2); // 경계
	return (1);
}

void draw_shape(t_info *info, char *canvas, t_shape *sh)
{
	int	x;
	int	y;
	int	is_in;

	y = 0;
	while (y < info->h)
	{
		x = 0;
		while (x < info->w)
		{
			is_in = in_rectangle((float)x, (float)y, sh);
			if ((sh->type == 'r' && is_in == 2) || (sh->type == 'R' && is_in))
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

	while ((ret = fscanf(fp, "%c %f %f %f %f %c\n", &sh.type, &sh.x, &sh.y, &sh.w, &sh.h, &sh.color)) == 6)
	{
		if (sh.w <= 0.00000000 || sh.h <= 0.00000000)
			return (0);
		if (sh.type != 'r' && sh.type != 'R')
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

int
	main(int argc, char **argv)
{
	FILE	*fp;
	t_info	info;
	char	*canvas;

	if (argc != 2)
		return (str_error("Error: argument\n"));
	if (!(fp = fopen(argv[1], "r")))
		return (str_error("Error: Operation file corrupted\n"));
	if (!(canvas = init_canvas(fp, &info)))
		return (clear_all(fp, NULL, "Error: Operation file corrupted\n"));
	if (!draw_shapes(fp, &info, canvas))
		return (clear_all(fp, canvas, "Error: Operation file corrupted\n"));
	print_canvas(&info, canvas);
	clear_all(fp, canvas, NULL);
	return (0);
}
