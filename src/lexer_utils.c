/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:34:00 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/07 22:09:40 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

static int	is_operator_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

static int	skip_quote(const char *s, int i)
{
	char	quote;

	quote = s[i];
	i++;
	while (s[i] && s[i] != quote)
		i++;
	if (s[i] == quote)
		i++;
	return (i);
}

int	token_len(const char *s, int i)
{
	int	start;

	start = i;
	if (is_operator_char(s[i]))
	{
		if ((s[i] == '<' || s[i] == '>') && s[i + 1] == s[i])
			return (2);
		return (1);
	}
	while (s[i] && s[i] != ' ' && !is_operator_char(s[i]))
	{
		if (s[i] == '\'' || s[i] == '\"')
			i = skip_quote(s, i);
		else
			i++;
	}
	return (i - start);
}

static int	skip_non_operator(const char *s, int i)
{
	while (s[i] && s[i] != ' ' && !is_operator_char(s[i]))
	{
		if (s[i] == '\'' || s[i] == '\"')
			i = skip_quote(s, i);
		else
			i++;
	}
	return (i);
}

int	count_tokens(const char *s)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		while (s[i] == ' ')
			i++;
		if (!s[i])
			break ;
		count++;
		if (is_operator_char(s[i]))
		{
			if ((s[i] == '<' || s[i] == '>') && s[i + 1] == s[i])
				i += 2;
			else
				i++;
		}
		else
			i = skip_non_operator(s, i);
	}
	return (count);
}
