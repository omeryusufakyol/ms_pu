/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/26 01:43:47 by oakyol            #+#    #+#             */
/*   Updated: 2024/10/28 21:30:35 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	unsigned char	*tmp_src;
	unsigned char	*tmp_dst;

	if (src == NULL && dst == NULL)
		return (dst);
	tmp_src = (unsigned char *)src;
	tmp_dst = (unsigned char *)dst;
	if (dst < src)
		ft_memcpy(dst, src, len);
	else if (dst > src)
		while (len-- > 0)
			tmp_dst[len] = tmp_src[len];
	return (dst);
}
