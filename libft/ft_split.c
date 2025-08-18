/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/26 21:54:19 by oakyol            #+#    #+#             */
/*   Updated: 2024/10/28 21:30:54 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_countword(const char *s, char c)
{
	size_t	count;

	count = 0;
	while (*s)
	{
		while (*s == c)
			s++;
		if (*s)
			count++;
		while (*s != c && *s)
			s++;
	}
	return (count);
}

static void	ft_free_split(char **res, int i)
{
	while (i--)
		free(res[i]);
	free(res);
}

static size_t	get_word_len(const char *s, char c)
{
	const char	*next_sep;
	size_t		word_len;

	next_sep = ft_strchr(s, c);
	if (next_sep)
	{
		word_len = (size_t)(next_sep - s);
	}
	else
	{
		word_len = ft_strlen(s);
	}
	return (word_len);
}

char	**ft_split(const char *s, char c)
{
	char	**res;
	size_t	word_len;
	int		i;

	res = (char **)malloc((ft_countword(s, c) + 1) * sizeof(char *));
	if (!res || !s)
		return (NULL);
	i = 0;
	while (*s)
	{
		while (*s == c && *s)
			s++;
		if (*s)
		{
			word_len = get_word_len(s, c);
			res[i] = ft_substr(s, 0, word_len);
			if (!res[i])
				return (ft_free_split(res, i), NULL);
			i++;
			s += word_len;
		}
	}
	return (res[i] = NULL, res);
}
