/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 21:45:15 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 16:10:55 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static void	bqt_scan_idx(const char *token, char *res, int *i, int *j)
{
	char	quote;

	while (token[*i])
	{
		if (token[*i] == '\'' || token[*i] == '"')
		{
			quote = token[*i];
			*i = *i + 1;
			while (token[*i] && token[*i] != quote)
			{
				res[*j] = token[*i];
				*j = *j + 1;
				*i = *i + 1;
			}
			if (token[*i] == quote)
				*i = *i + 1;
		}
		else
		{
			res[*j] = token[*i];
			*j = *j + 1;
			*i = *i + 1;
		}
	}
}

char	*bash_quote_trim(const char *token, t_ms *ms)
{
	char	*res;
	int		i;
	int		j;

	res = gc_malloc(ms, ft_strlen(token) + 1);
	if (!res)
	{
		write(2, "minishell: bash_quote_trim allocation failed\n", 34);
		return (NULL);
	}
	i = 0;
	j = 0;
	bqt_scan_idx(token, res, &i, &j);
	res[j] = '\0';
	return (res);
}

static int	handle_parse_block(t_cmd *cmd, char **tokens, int *i)
{
	if (!cmd->parse_block)
		return (0);
	if (!ft_strcmp(tokens[*i], "<<"))
		*i = *i + 2;
	else if (!ft_strcmp(tokens[*i], "<")
		|| !ft_strcmp(tokens[*i], ">")
		|| !ft_strcmp(tokens[*i], ">>"))
	{
		*i = *i + 1;
		if (tokens[*i])
			*i = *i + 1;
	}
	else
		*i = *i + 1;
	return (1);
}

/* ortak doğrulama: eksik isim -> 1, boş clean -> 0 (i ilerler), başarı -> 2 */
static int	validate_filename(char *filename, t_cmd *cmd, t_ms *ms, int *i)
{
	char	*clean;

	if (!filename)
	{
		ms->last_exit = 2;
		return (1);
	}
	clean = bash_quote_trim(filename, ms);
	if (clean[0] == '\0')
	{
		ms->last_exit = 1;
		cmd->redirect_error = 1;
		*i = *i + 1;
		return (0);
	}
	return (2);
}

static int	handle_in_redir(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	char	*filename;
	int		rc;

	*i = *i + 1;
	filename = tokens[*i];
	rc = validate_filename(filename, cmd, ms, i);
	if (rc != 2)
		return (rc);
	if (!cmd->infile)
	{
		cmd->infile = gc_strdup(ms, filename);
		cmd->seen_input = 1;
	}
	*i = *i + 1;
	return (0);
}

static int	handle_out_helper(t_cmd *cmd)
{
	if (!cmd->seen_input && cmd->heredoc)
	{
		cmd->def_out_pending = 1;
		cmd->def_out_path = cmd->outfile;
		cmd->def_out_append = 0;
		return (1);
	}
	return (0);
}

static int	handle_out_trunc(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	int		fd;
	char	*filename;
	int		rc;

	*i = *i + 1;
	filename = tokens[*i];
	rc = validate_filename(filename, cmd, ms, i);
	if (rc != 2)
		return (rc);
	cmd->outfile = gc_strdup(ms, filename);
	cmd->append = 0;
	if (!handle_out_helper(cmd) && (!cmd->heredoc && !cmd->seen_input))
	{
		fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd >= 0)
			close(fd);
	}
	*i = *i + 1;
	return (0);
}

static int	handle_out_append(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	int		fd;
	char	*filename;
	int		rc;

	*i = *i + 1;
	filename = tokens[*i];
	rc = validate_filename(filename, cmd, ms, i);
	if (rc != 2)
		return (rc);
	cmd->outfile = gc_strdup(ms, filename);
	cmd->append = 1;
	if (!handle_out_helper(cmd) && (!cmd->heredoc && !cmd->seen_input))
	{
		fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd >= 0)
			close(fd);
	}
	*i = *i + 1;
	return (0);
}

static int	parser_handle_heredoc(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	char	*clean;

	clean = bash_quote_trim(tokens[*i + 1], ms);
	if (!clean)
	{
		ms->last_exit = 1;
		return (1);
	}
	add_heredoc(cmd, clean, ms);
	ms->heredoc_index = ms->heredoc_index + 1;
	cmd->heredoc = 1;
	*i = *i + 2;
	return (0);
}

static int	dispatch_redirect(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	if (!ft_strcmp(tokens[*i], "<"))
		return (handle_in_redir(cmd, tokens, i, ms));
	if (!ft_strcmp(tokens[*i], ">"))
		return (handle_out_trunc(cmd, tokens, i, ms));
	if (!ft_strcmp(tokens[*i], ">>"))
		return (handle_out_append(cmd, tokens, i, ms));
	if (!ft_strcmp(tokens[*i], "<<"))
		return (parser_handle_heredoc(cmd, tokens, i, ms));
	*i = *i + 1;
	return (0);
}

int	parse_redirect(t_cmd *cmd, char **tokens, int *i, t_ms *ms)
{
	if (handle_parse_block(cmd, tokens, i))
		return (0);
	return (dispatch_redirect(cmd, tokens, i, ms));
}

int	is_quoted_operator(const char *raw_input, const char *op)
{
	int		i;
	int		inside_single;
	int		inside_double;
	size_t	op_len;

	i = 0;
	inside_single = 0;
	inside_double = 0;
	op_len = ft_strlen(op);
	while (raw_input[i])
	{
		if (raw_input[i] == '\'' && !inside_double)
			inside_single = !inside_single;
		else if (raw_input[i] == '"' && !inside_single)
			inside_double = !inside_double;
		else if (!inside_single && !inside_double
			&& !ft_strncmp(&raw_input[i], op, op_len))
			return (0);
		else if ((inside_single || inside_double)
			&& !ft_strncmp(&raw_input[i], op, op_len))
			return (1);
		i++;
	}
	return (0);
}

static int	print_leading_quoted_redirect_err(t_ms *ms, const char *tok)
{
	write(2, "minishell: ", 11);
	write(2, tok, ft_strlen(tok));
	write(2, ": command not found\n", 21);
	ms->last_exit = 127;
	return (-1);
}

static void	parser_inner_loop(t_cmd *cur, char **tokens, int *i, t_ms *ms)
{
	while (tokens[*i])
	{
		if (!ft_strcmp(tokens[*i], "|")
			&& !is_quoted_operator_parser(ms->raw_input, *i))
			break ;
		if (is_redirect(tokens[*i])
			&& !is_quoted_operator_parser(ms->raw_input, *i))
		{
			parse_redirect(cur, tokens, i, ms);
			continue ;
		}
		*i = *i + 1;
	}
}

t_cmd	*parser(char **tokens, t_ms *ms)
{
	t_cmd	*cmds;
	t_cmd	*current;
	int		i;
	int		cmd_start;

	cmds = NULL;
	i = 0;
	while (tokens[i])
	{
		current = init_cmd(ms);
		cmd_start = i;
		if (tokens[i] && is_redirect(tokens[i])
			&& is_quoted_operator_parser(ms->raw_input, i))
			if (print_leading_quoted_redirect_err(ms, tokens[i]) < 0)
				return (NULL);
		parser_inner_loop(current, tokens, &i, ms);
		current->args = copy_args(tokens, cmd_start, i, ms);
		if (current->heredoc_delims || current->args)
			add_cmd(&cmds, current);
		if (tokens[i])
			i = i + 1;
	}
	return (cmds);
}
