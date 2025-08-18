/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_export.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:18:28 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 13:06:35 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static int	env_len(char **env)
{
	int	i;

	i = 0;
	while (env && env[i])
		i++;
	return (i);
}

static int	key_in_env(char **env, const char *key)
{
	int	i;
	int	len;

	len = ft_strlen(key);
	i = 0;
	while (env && env[i])
	{
		if (!ft_strncmp(env[i], key, len) && env[i][len] == '=')
			return (1);
		i++;
	}
	return (0);
}

static void	copy_env(char **dst, char **src, t_ms *ms, int *i)
{
	int	j;

	j = 0;
	while (src && src[j])
		dst[(*i)++] = gc_strdup(ms, src[j++]);
}

static void	copy_export(char **dst, char **export, int *i, t_ms *ms)
{
	int		j;
	char	*key;

	j = 0;
	while (export && export[j])
	{
		key = export[j];
		if (!key_in_env(dst, key))
			dst[(*i)++] = gc_strdup(ms, key);
		j++;
	}
}

static char	**combine_env_export(t_ms *ms, char **env, char **export_only)
{
	char	**combined;
	int		total;
	int		i;

	total = env_len(env) + env_len(export_only);
	combined = gc_malloc(ms, sizeof(char *) * (total + 1));
	if (!combined)
		return (NULL);
	i = 0;
	copy_env(combined, env, ms, &i);
	copy_export(combined, export_only, &i, ms);
	combined[i] = NULL;
	return (combined);
}

static	void	sort_env_copy(char **copy, int len)
{
	int		i;
	int		j;
	char	*tmp;

	i = 0;
	while (i < len - 1)
	{
		j = i + 1;
		while (j < len)
		{
			if (ft_strcmp(copy[i], copy[j]) > 0)
			{
				tmp = copy[i];
				copy[i] = copy[j];
				copy[j] = tmp;
			}
			j++;
		}
		i++;
	}
}

void	print_export(t_ms *ms)
{
	char	**all;
	int		len;
	int		i;
	char	*eq;

	all = combine_env_export(ms, ms->env, ms->export_only);
	if (!all)
		return ;
	len = env_len(all);
	sort_env_copy(all, len);
	i = 0;
	while (i < len)
	{
		eq = ft_strchr(all[i], '=');
		if (eq)
		{
			*eq = '\0';
			printf("declare -x %s=\"%s\"\n", all[i], eq + 1);
			*eq = '=';
		}
		else
			printf("declare -x %s\n", all[i]);
		i++;
	}
}
