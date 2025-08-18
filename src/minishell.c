/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:46:57 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:37:06 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static void	process_line(const char *line, t_ms *ms)
{
	char	**tokens;
	char	**expanded;
	t_cmd	*cmds;

	if (line[0] == '\0')
		return ;
	tokens = lexer(line, ms);
	if (!tokens || check_syntax(tokens, ms))
		return ;
	if (contains_heredoc(tokens))
		cmds = parser(tokens, ms);
	else
	{
		expanded = expand_tokens(tokens, ms);
		if (!expanded || check_syntax(expanded, ms))
			return ;
		cmds = parser(expanded, ms);
	}
	if (cmds)
		execute(cmds, ms);
}

static void	process_multiline_input(const char *input, t_ms *ms)
{
	char	**lines;
	int		i;

	lines = ft_split(input, '\n');
	if (!lines)
		return ;
	i = 0;
	while (lines[i])
	{
		process_line(lines[i], ms);
		i++;
	}
}

static t_cmd	*parse_tokens_or_expanded(char **tokens, t_ms *ms)
{
	char	**expanded;
	t_cmd	*cmds;

	if (contains_heredoc(tokens))
	{
		cmds = parser(tokens, ms);
		return (cmds);
	}
	expanded = expand_tokens(tokens, ms);
	if (!expanded)
		return (NULL);
	if (check_syntax(expanded, ms))
		return (NULL);
	cmds = parser(expanded, ms);
	return (cmds);
}

static int	process_after_read(char *raw, t_ms *ms)
{
	char	*input;
	char	**tokens;
	t_cmd	*cmds;

	input = gc_strdup(ms, raw);
	free(raw);
	ms->raw_input = input;
	if (ft_strchr(input, '\n'))
	{
		process_multiline_input(input, ms);
		return (1);
	}
	tokens = lexer(input, ms);
	if (!tokens || check_syntax(tokens, ms))
		return (1);
	cmds = parse_tokens_or_expanded(tokens, ms);
	if (!cmds)
		return (1);
	execute(cmds, ms);
	return (0);
}

void	mini_loop(t_ms *ms)
{
	char	*raw;

	while (1)
	{
		ms->heredoc_index = 0;
		raw = readline("minishell$ ");
		if (!raw)
		{
			write(1, "\nexit\n", 6);
			break ;
		}
		if (g_heredoc_sigint)
		{
			ms->last_exit = 130;
			g_heredoc_sigint = 0;
		}
		if (raw[0] != '\0')
			add_history(raw);
		if (process_after_read(raw, ms))
			continue ;
	}
	(void)ms;
}

// BU SAYFAYA DIKKAT / NORM'DA BOZULMUS OLABILIR