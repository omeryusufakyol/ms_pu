/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 23:42:04 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/12 20:50:20 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char	*join_path(t_ms *ms, const char *dir, const char *cmd)
{
	char	*full;
	int		len;
	int		i;
	int		j;

	len = ft_strlen(dir) + ft_strlen(cmd) + 2;
	full = gc_malloc(ms, len);
	if (!full)
		return (NULL);
	i = 0;
	while (dir[i])
	{
		full[i] = dir[i];
		i++;
	}
	full[i++] = '/';
	j = 0;
	while (cmd[j])
		full[i++] = cmd[j++];
	full[i] = '\0';
	return (full);
}

static char	*search_in_paths(t_ms *ms, char **paths, char *cmd)
{
	char	*path;
	int		i;

	i = 0;
	while (paths[i])
	{
		if (paths[i][0] == '\0')
			path = join_path(ms, ".", cmd);
		else
			path = join_path(ms, paths[i], cmd);
		if (path && access(path, X_OK) == 0)
			return (path);
		i++;
	}
	return (NULL);
}

char	*find_path(t_ms *ms, char *cmd, char **env)
{
	char	**paths;
	char	*path;

	if (!cmd || !*cmd)
		return (NULL);
	if (ft_strchr(cmd, '/'))
		return (gc_strdup(ms, cmd));
	while (*env && ft_strncmp(*env, "PATH=", 5))
		env++;
	if (!*env || (*(*env + 5) == '\0'))
	{
		path = join_path(ms, ".", cmd);
		if (path && access(path, X_OK) == 0)
			return (path);
		return (NULL);
	}
	paths = gc_split(ms, *env + 5, ':');
	return (search_in_paths(ms, paths, cmd));
}
