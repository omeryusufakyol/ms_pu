/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 15:43:54 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:49:01 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"


int	is_redirect(const char *token)
{
	return (!ft_strcmp(token, "<")
		|| !ft_strcmp(token, ">")
		|| !ft_strcmp(token, ">>")
		|| !ft_strcmp(token, "<<"));
}

static void	skip_spaces(const char *s, int *i)
{
	while (s[*i] && (s[*i] == ' ' || s[*i] == '\t'))
		*i = *i + 1;
}

void	advance_quoted_chunk(const char *s, int *i)
{
	char	q;

	q = s[*i];
	*i = *i + 1;
	while (s[*i] && s[*i] != q)
		*i = *i + 1;
	if (s[*i] == q)
		*i = *i + 1;
}

static int	advance_token(const char *s, int *i)
{
	if (s[*i] == '\'' || s[*i] == '"')
	{
		advance_quoted_chunk(s, i);
		return (1);
	}
	advance_unquoted_until_space(s, i);
	return (0);
}

int	is_quoted_operator_parser(const char *raw, int target_idx)
{
	int	i;
	int	count;
	int	quoted_start;

	i = 0;
	count = 0;
	while (raw[i])
	{
		skip_spaces(raw, &i);
		if (!raw[i])
			break ;
		quoted_start = advance_token(raw, &i);
		if (count == target_idx)
			return (quoted_start);
		count = count + 1;
	}
	return (0);
}
