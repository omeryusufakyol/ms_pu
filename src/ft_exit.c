/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:07:13 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 15:51:54 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

static int	is_numeric(const char *str)
{
	int	i;

	i = 0;
	if (!str)
		return (0);
	if (str[i] == '-' || str[i] == '+')
		i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static long	ft_atol_safe(const char *str, int *overflow, int sign)
{
	long	res;
	int		digit;

	res = 0;
	*overflow = 0;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			sign = -1;
		str++;
	}
	while (*str)
	{
		if (*str < '0' || *str > '9')
			break ;
		digit = *str - '0';
		if (res > (LONG_MAX - digit) / 10)
		{
			*overflow = 1;
			break ;
		}
		res = res * 10 + digit;
		str++;
	}
	return (res * sign);
}

static int	handle_exit_error(t_ms *ms, const char *msg, int exit_immediately)
{
	write(2, msg, ft_strlen(msg));
	gc_free_all(ms);
	if (exit_immediately)
		exit(2);
	return (1);
}

int	ft_exit(char **args, t_ms *ms)
{
	long		code;
	char		*trimmed;
	int			overflow;

	if (isatty(STDIN_FILENO))
		write(1, "exit\n", 5);
	if (!args[1])
	{
		gc_free_all(ms);
		exit(ms->last_exit);
	}
	trimmed = gc_strtrim(ms, args[1], " \t\n\r\v\f");
	if (!is_numeric(trimmed))
		return (handle_exit_error(ms,
				"minishell: exit: numeric argument required\n", 1));
	if (args[2])
		return (write(2, "minishell: exit: too many arguments\n", 36), 1);
	code = ft_atol_safe(trimmed, &overflow, 1);
	if (overflow)
		return (handle_exit_error(ms,
				"minishell: exit: numeric argument required\n", 1));
	gc_free_all(ms);
	exit((unsigned char)code);
}
