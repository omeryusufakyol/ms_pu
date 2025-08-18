/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:46:07 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:39:39 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

void	add_cmd(t_cmd **cmds, t_cmd *new_cmd)
{
	t_cmd	*tmp;

	if (!*cmds)
	{
		*cmds = new_cmd;
		return ;
	}
	tmp = *cmds;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new_cmd;
}

int	count_args(char **tokens, int i)
{
	int	count;

	count = 0;
	while (i > 0 && tokens[i - 1] && ft_strcmp(tokens[i - 1], "|"))
	{
		if (!ft_strcmp(tokens[i - 1], "<")
			|| !ft_strcmp(tokens[i - 1], ">")
			|| !ft_strcmp(tokens[i - 1], ">>")
			|| !ft_strcmp(tokens[i - 1], "<<"))
			i -= 2;
		else
		{
			count++;
			i--;
		}
	}
	return (count);
}

static int	count_non_redir_args(char **tokens, int start, int end, t_ms *ms)
{
	int	count;
	int	i;

	count = 0;
	i = start;
	while (i < end)
	{
		if ((is_redirect(tokens[i])
				&& !is_quoted_operator(ms->raw_input, tokens[i])))
			i += 2;
		else
		{
			count++;
			i++;
		}
	}
	return (count);
}

static int	should_skip_token(char **tokens, int i, t_ms *ms)
{
	if (is_redirect(tokens[i]) && !is_quoted_operator(ms->raw_input, tokens[i]))
		return (1);
	if (i > 0 && !ft_strcmp(tokens[i - 1], "<<")
		&& !is_quoted_operator(ms->raw_input, tokens[i - 1]))
		return (1);
	return (0);
}

char	**copy_args(char **tokens, int start, int end, t_ms *ms)
{
	char	**args;
	int		i;
	int		j;
	int		count;

	count = count_non_redir_args(tokens, start, end, ms);
	args = gc_malloc(ms, sizeof(char *) * (count + 1));
	if (!args)
		return (NULL);
	i = start;
	j = 0;
	while (i < end)
	{
		if (should_skip_token(tokens, i, ms))
		{
			if (is_redirect(tokens[i]))
				i += 2;
			else
				i++;
			continue ;
		}
		args[j++] = bash_quote_trim(tokens[i++], ms);
	}
	args[j] = NULL;
	return (args);
}
