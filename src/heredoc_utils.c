/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:22:34 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 17:14:15 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../libft/libft.h"
#include "../include/minishell.h"
#include <stdlib.h>

static void	append_env_exit_status(t_ms *ms, char **res, int *i)
{
	char	*val;

	*i += 2;
	val = gc_itoa(ms, ms->last_exit);
	*res = gc_strjoin_free(ms, *res, val);
}

static void	append_env_var(t_ms *ms, char **res, char *line, int *i)
{
	char	*temp;
	char	*val;
	int		start;

	(*i)++;
	start = *i;
	while (line[*i] && (ft_isalnum(line[*i]) || line[*i] == '_'))
		(*i)++;
	temp = gc_strndup(ms, &line[start], *i - start);
	val = get_env_value(ms, temp);
	*res = gc_strjoin_free(ms, *res, val);
}

static void	append_text(t_ms *ms, char **res, char *line, int *i)
{
	char	*temp;
	int		start;

	start = *i;
	while (line[*i] && line[*i] != '$')
		(*i)++;
	temp = gc_strndup(ms, &line[start], *i - start);
	*res = gc_strjoin_free(ms, *res, temp);
}

char	*expand_heredoc_line_envonly(char *line, t_ms *ms)
{
	char	*result;
	int		i;

	if (!line)
		return (NULL);
	result = gc_strdup(ms, "");
	if (!result)
		return (NULL);
	i = 0;
	while (line[i])
	{
		if (line[i] == '$' && line[i + 1] == '?')
			append_env_exit_status(ms, &result, &i);
		else if (line[i] == '$'
			&& (ft_isalpha(line[i + 1]) || line[i + 1] == '_'))
			append_env_var(ms, &result, line, &i);
		else
			append_text(ms, &result, line, &i);
	}
	return (result);
}
