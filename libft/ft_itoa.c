/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 17:59:41 by oakyol            #+#    #+#             */
/*   Updated: 2024/10/28 21:30:20 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	int_len(int n)
{
	int	x;

	x = 0;
	if (n == 0)
		return (1);
	if (n < 0)
		x++;
	while (n != 0)
	{
		n /= 10;
		x++;
	}
	return (x);
}

char	*ft_itoa(int nbr)
{
	long	temp;
	char	*str;
	int		x;

	temp = nbr;
	x = int_len(temp);
	str = malloc(x + 1);
	if (!str)
		return (NULL);
	if (nbr < 0)
	{
		temp *= -1;
	}
	str[x] = '\0';
	while (x--)
	{
		str[x] = temp % 10 + '0';
		temp /= 10;
	}
	if (nbr < 0)
		str[0] = '-';
	return (str);
}
