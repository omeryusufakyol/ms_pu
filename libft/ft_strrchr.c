/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/26 02:10:35 by oakyol            #+#    #+#             */
/*   Updated: 2024/10/28 18:04:17 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	int		i;
	char	*res;
	char	ch;

	ch = (char)c;
	res = NULL;
	i = 0;
	while (s[i])
	{
		if (s[i] == ch)
			res = (char *)&s[i];
		i++;
	}
	if (s[i] == ch)
		res = (char *)&s[i];
	return (res);
}
