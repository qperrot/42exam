/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <wpark@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/04 20:24:21 by wpark             #+#    #+#             */
/*   Updated: 2019/12/18 15:55:35 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
# define MAX(a, b) (a > b ? a : b)
# define MIN(a, b) (a < b ? a : b)

int is_convert(char c)
{
	if (c == 's' || c == 'd' || c == 'x')
		return (1);
	return (0);
}

int ft_strlen(char *s)
{
	int i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

int get_minw(char *form)
{
	int minw = 0;

	while (*form && *form != '.' && !is_convert(*form))
	{
		minw = minw * 10 + *form - '0';
		form++;
	}
	return (minw);
}

char	get_prec(char *form)
{
	int prec = -1;

	while (*form && *form != '.' && !is_convert(*form))
		form++;
	if (*form == '.')
	{
		form++;
		prec = 0;
		while (*form && !is_convert(*form))
		{
			prec = prec * 10 + *form - '0';
			form++;
		}
	}
	return (prec);
}

char	get_conv(char *form)
{
	while (*form)
	{
		if (is_convert(*form))
			return (*form);
		form++;
	}
	return (0);
}

int get_next(char *form)
{
	char *f;
	
	f = form;
	while (*form && !is_convert(*form))
		form++;
	return (form - f);
}

int	put_char(char c, int i)
{
	int ret = i;
	
	if (ret < 0)
		ret = 0;
	while (i-- > 0)
		write(1, &c, 1);
	return (ret);
}

int calc_digits(unsigned int u, int b)
{
	int	len = 0;

	if (u == 0)
		return (1);
	while (u != 0)
	{
		u /= b;
		len += 1;
	}
	return (len);
}

char *ft_utoa_base(unsigned int u, char *base)
{
	int b = ft_strlen(base);
	int len = calc_digits(u, b);
	char *res;

	if (!(res = malloc(len + 1)))
		return (0);
	res[len] = '\0';
	while (--len >= 0)
	{
		res[len] = base[u % b];
		u = u / b;
	}
	return (res);
}

int print_str(int minw, int prec, char *s)
{
	int res = 0;
	int len;
	int plen;
	int slen;

	if (!s)
		s = "(null)";
	len = ft_strlen(s);
	if (prec != -1)
		plen = MIN(prec, len);
	else
		plen = len;
	slen = MAX(minw, plen);
	res += put_char(' ', slen - plen);
	res += write(1, s, plen);
	return (res);
}

int print_dec(int minw, int prec, int n)
{
	int res = 0;
	unsigned int u = (n < 0 ? -n : n);
	int sign = (n < 0 ? 1 : 0);
	char *nb = ft_utoa_base(u, "0123456789");
	int len;
	int plen;
	int zlen;
	int slen;

	len = ft_strlen(nb);
	if (prec != -1)
		plen = MAX(len, prec);
	else
		plen = len;
	if (prec == 0 && u == 0)
		plen = 0;
	zlen = plen - len;
	slen = MAX(plen + sign, minw);
	res += put_char(' ', slen - sign - plen);
	res += put_char('-', sign);
	res += put_char('0', zlen);
	if (prec != 0 || u != 0)
		res += write(1, nb, len);
	return (res);
}

int print_hex(int minw, int prec, unsigned int u)
{
	int res = 0;
	int sign = 0;
	char *nb = ft_utoa_base(u, "0123456789abcdef");
	int len;
	int plen;
	int zlen;
	int slen;

	len = ft_strlen(nb);
	if (prec != -1)
		plen = MAX(len, prec);
	else
		plen = len;
	if (prec == 0 && u == 0)
		plen = 0;
	zlen = plen - len;
	slen = MAX(plen + sign, minw);
	res += put_char(' ', slen - sign - plen);
	res += put_char('-', sign);
	res += put_char('0', zlen);
	if (prec != 0 || u != 0)
		res += write(1, nb, len);
	return (res);
}

int ft_printf(const char *f, ...)
{
	int minw;
	int prec;
	char conv;
	va_list	ap;
	int res = 0;
	char *form;

	form = (char*)f;
	va_start(ap, f);
	while (*form != '\0')
	{
		if (*form == '%')
		{
			form++;
			minw = get_minw(form);
			prec = get_prec(form);
			conv = get_conv(form);
			form += get_next(form);
			if (conv == 's')
				res += print_str(minw, prec, va_arg(ap, char*));
			if (conv == 'd')
				res += print_dec(minw, prec, va_arg(ap, int));
			if (conv == 'x')
				res += print_hex(minw, prec, va_arg(ap, unsigned int));
		}
		else
			res += write(1, &*form, 1);
		form++;
	}
	return (res);
}