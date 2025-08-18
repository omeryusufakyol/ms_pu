/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:19:04 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/08 21:11:38 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int	ft_pwd(t_ms *ms)
{
	char	*tmp;
	char	*buf;

	tmp = getcwd(NULL, 0);
	if (!tmp)
	{
		perror("pwd");
		return (1);
	}
	buf = gc_strdup(ms, tmp);
	free(tmp);
	if (!buf)
		return (1);
	printf("%s\n", buf);
	return (0);
}
