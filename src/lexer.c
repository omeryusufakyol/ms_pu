/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:38:55 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/01 16:38:36 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static char	*extract_token(const char *s, int *i, t_ms *ms)
{
	int		len;
	int		start;
	char	*token;

	while (s[*i] == ' ')
		(*i)++;
	start = *i;
	len = token_len(s, *i);
	*i += len;
	token = gc_strndup(ms, &s[start], len);
	if (!token)
		return (NULL);
	return (token);
}

char	**lexer(const char *input, t_ms *ms)
{
	int		count;
	int		i;
	int		j;
	char	**tokens;

	count = count_tokens(input);
	tokens = gc_malloc(ms, sizeof(char *) * (count + 1));
	if (!tokens)
		return (NULL);
	i = 0;
	j = 0;
	while (j < count)
	{
		tokens[j] = extract_token(input, &i, ms);
		if (!tokens[j])
			return (NULL);
		j++;
	}
	tokens[j] = NULL;
	return (tokens);
}
