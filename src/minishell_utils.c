/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 15:36:42 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:37:10 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../libft/libft.h"

int	contains_heredoc(char **tokens)
{
	int	i;

	i = 0;
	while (tokens[i])
	{
		if (!ft_strcmp(tokens[i], "<<"))
			return (1);
		i++;
	}
	return (0);
}
