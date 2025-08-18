/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax_check.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 21:45:15 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 21:38:23 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static int	is_operator(const char *token)
{
	return (!ft_strcmp(token, "|")
		|| !ft_strcmp(token, "<")
		|| !ft_strcmp(token, ">")
		|| !ft_strcmp(token, ">>")
		|| !ft_strcmp(token, "<<"));
}

static int	handle_syntax_error(t_ms *ms, char *msg)
{
	ft_putendl_fd(msg, 2);
	ms->last_exit = 2;
	return (1);
}

static int	check_syntax_tokens(char **tokens, t_ms *ms, int i)
{
	if (is_operator(tokens[0]) && !is_quoted_operator(ms->raw_input, tokens[0]))
	{
		if (!ft_strcmp(tokens[0], "|"))
			return (handle_syntax_error(ms,
					"syntax error near unexpected token `|'"));
		if (!tokens[1] || is_operator(tokens[1]))
			return (handle_syntax_error(ms,
					"syntax error near unexpected token `newline'"));
	}
	while (tokens[i])
	{
		if (is_operator(tokens[i])
			&& !is_quoted_operator(ms->raw_input, tokens[i]))
		{
			if (!tokens[i + 1])
				return (handle_syntax_error(ms,
						"syntax error near unexpected token `newline'"));
			if (is_operator(tokens[i + 1]) && ft_strcmp(tokens[i], "|"))
				return (handle_syntax_error(ms,
						"syntax error near unexpected token `newline'"));
		}
		i++;
	}
	return (0);
}

int	check_syntax(char **tokens, t_ms *ms)
{
	int	index;

	index = 0;
	if (!tokens || !tokens[0])
		return (0);
	return (check_syntax_tokens(tokens, ms, index));
}
