/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils3.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 15:47:18 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 16:11:01 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	advance_unquoted_until_space(const char *s, int *i)
{
	while (s[*i] && s[*i] != ' ' && s[*i] != '\t')
	{
		if (s[*i] == '\'' || s[*i] == '"')
			advance_quoted_chunk(s, i);
		else
			*i = *i + 1;
	}
}
