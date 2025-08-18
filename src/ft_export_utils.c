/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:12:54 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 16:34:05 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

int	has_equal(const char *s)
{
	while (*s && *s != '=')
		s++;
	return (*s == '=');
}

int	update_if_exists(t_ms *ms, char *clean)
{
	int		i;
	size_t	len;
	char	*eq;

	eq = ft_strchr(clean, '=');
	if (!eq)
		return (0);
	len = eq - clean + 1;
	i = 0;
	if (!ms->env)
		return (0);
	while (ms->env[i])
	{
		if (!ft_strncmp(ms->env[i], clean, len))
		{
			ms->env[i] = clean;
			return (1);
		}
		i++;
	}
	return (0);
}

void	add_to_export_only(t_ms *ms, const char *arg, int index)
{
	int		len;
	char	**new;

	while (ms->export_only && ms->export_only[index])
	{
		if (ft_strcmp(ms->export_only[index], arg) == 0)
			return ;
		index++;
	}
	len = 0;
	while (ms->export_only && ms->export_only[len])
		len++;
	new = gc_malloc(ms, sizeof(char *) * (len + 2));
	if (!new)
		return ;
	index = 0;
	while (index < len)
	{
		new[index] = ms->export_only[index];
		index++;
	}
	new[index] = gc_strdup(ms, arg);
	new[index + 1] = NULL;
	ms->export_only = new;
}

void	remove_from_export_only(t_ms *ms, const char *arg)
{
	int		i;
	int		j;
	int		len;
	char	**new;

	if (!ms->export_only)
		return ;
	len = 0;
	while (ms->export_only[len])
		len++;
	new = gc_malloc(ms, sizeof(char *) * (len + 1));
	if (!new)
		return ;
	i = 0;
	j = 0;
	while (ms->export_only[i])
	{
		if (ft_strcmp(ms->export_only[i], arg) != 0)
			new[j++] = ms->export_only[i];
		i++;
	}
	new[j] = NULL;
	ms->export_only = new;
}
