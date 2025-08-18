/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:20:02 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 17:49:50 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <stdlib.h>
#include <unistd.h>

static int	is_valid_identifier(const char *s)
{
	int	i;

	if (!s || !(s[0]) || (s[0] >= '0' && s[0] <= '9'))
		return (0);
	i = 0;
	while (s[i])
	{
		if (!(s[i] == '_' || (s[i] >= 'A' && s[i] <= 'Z')
				|| (s[i] >= 'a' && s[i] <= 'z')
				|| (s[i] >= '0' && s[i] <= '9')))
			return (0);
		i++;
	}
	return (1);
}

static int	find_var_index(char **env, const char *name)
{
	int	i;
	int	name_len;

	if (!env || !name)
		return (-1);
	name_len = ft_strlen(name);
	i = 0;
	while (env[i])
	{
		if (ft_strncmp(env[i], name, name_len) == 0 && env[i][name_len] == '=')
			return (i);
		i++;
	}
	return (-1);
}

static char	**create_new_env_without_var(t_ms *ms, char **env, int index_skip)
{
	int		i;
	int		k;
	int		len;
	char	**new_env;

	len = 0;
	while (env && env[len])
		len++;
	new_env = gc_malloc(ms, sizeof(char *) * len);
	if (!new_env)
		return (NULL);
	i = 0;
	k = 0;
	while (env[i])
	{
		if (i != index_skip)
			new_env[k++] = env[i];
		i++;
	}
	new_env[k] = NULL;
	return (new_env);
}

void	remove_var(t_ms *ms, const char *name)
{
	int		index;

	if (!ms || !ms->env || !name)
		return ;
	index = find_var_index(ms->env, name);
	if (index == -1)
		return ;
	ms->env = create_new_env_without_var(ms, ms->env, index);
}

int	ft_unset(char **args, t_ms *ms)
{
	int	i;

	i = 1;
	while (args[i])
	{
		if (!is_valid_identifier(args[i]))
		{
			write(2, "minishell: unset: not a valid identifier\n", 42);
			return (1);
		}
		remove_var(ms, args[i]);
		remove_from_export_only(ms, args[i]);
		i++;
	}
	return (0);
}
