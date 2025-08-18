/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gc.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 15:36:23 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 13:05:20 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static size_t	ft_strnlen(const char *s, size_t maxlen)
{
	size_t	i;

	i = 0;
	while (i < maxlen && s[i])
		i++;
	return (i);
}

void	gc_init(t_ms *ms)
{
	ms->gc_list = NULL;
}

void	*gc_malloc(t_ms *ms, size_t size)
{
	void		*ptr;
	t_gc_node	*node;

	ptr = ft_calloc(1, size);
	if (!ptr)
		return (NULL);
	node = malloc(sizeof(t_gc_node));
	if (!node)
	{
		free(ptr);
		return (NULL);
	}
	node->ptr = ptr;
	node->next = ms->gc_list;
	ms->gc_list = node;
	return (ptr);
}

void	gc_free_all(t_ms *ms)
{
	t_gc_node	*tmp;

	while (ms->gc_list)
	{
		tmp = ms->gc_list;
		ms->gc_list = ms->gc_list->next;
		free(tmp->ptr);
		free(tmp);
	}
	ms->gc_list = NULL;
}

char	*gc_strdup(t_ms *ms, const char *s)
{
	size_t	len;
	char	*copy;

	if (!s)
		return (NULL);
	len = ft_strlen(s) + 1;
	copy = gc_malloc(ms, len);
	if (!copy)
		return (NULL);
	ft_memcpy(copy, s, len);
	return (copy);
}

char	*gc_strndup(t_ms *ms, const char *s, size_t n)
{
	size_t	len;
	char	*copy;

	len = ft_strnlen(s, n);
	copy = gc_malloc(ms, len + 1);
	if (!copy)
		return (NULL);
	ft_memcpy(copy, s, len);
	copy[len] = '\0';
	return (copy);
}

char	*gc_strjoin(t_ms *ms, const char *s1, const char *s2)
{
	size_t	len1;
	size_t	len2;
	char	*joined;

	len1 = ft_strlen(s1);
	len2 = ft_strlen(s2);
	joined = gc_malloc(ms, len1 + len2 + 1);
	if (!joined)
		return (NULL);
	ft_memcpy(joined, s1, len1);
	ft_memcpy(joined + len1, s2, len2);
	joined[len1 + len2] = '\0';
	return (joined);
}

static int	ptr_in_gc(void *ptr, t_ms *ms)
{
	t_gc_node	*node;

	node = ms->gc_list;
	while (node)
	{
		if (node->ptr == ptr)
			return (1);
		node = node->next;
	}
	return (0);
}

static void	safe_free(t_ms *ms, void *ptr)
{
	if (!ptr)
		return ;
	if (!ptr_in_gc(ptr, ms))
		free(ptr);
}

char	*gc_strjoin_free(t_ms *ms, char *s1, char *s2)
{
	char	*joined;

	joined = gc_strjoin(ms, s1, s2);
	safe_free(ms, s1);
	return (joined);
}

char	*gc_strjoin_free_both(t_ms *ms, char *s1, char *s2)
{
	char	*joined;

	joined = gc_strjoin(ms, s1, s2);
	safe_free(ms, s1);
	safe_free(ms, s2);
	return (joined);
}

static size_t	count_words(const char *s, char c)
{
	size_t	i;
	size_t	count;

	i = 0;
	count = 0;
	while (s[i])
	{
		while (s[i] && s[i] == c)
			i++;
		if (s[i])
		{
			count++;
			while (s[i] && s[i] != c)
				i++;
		}
	}
	return (count);
}

char	**gc_split(t_ms *ms, const char *s, char c)
{
	size_t	i;
	size_t	j;
	size_t	start;
	size_t	count;
	char	**result;

	if (!s)
		return (NULL);
	count = count_words(s, c);
	result = gc_malloc(ms, sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (s[i] && j < count)
	{
		while (s[i] && s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		result[j++] = gc_strndup(ms, &s[start], i - start);
	}
	result[j] = NULL;
	return (result);
}

char	*gc_strtrim(t_ms *ms, const char *s1, const char *set)
{
	size_t	start;
	size_t	end;
	char	*trimmed;

	if (!s1 || !set)
		return (NULL);
	start = 0;
	while (s1[start] && ft_strchr(set, s1[start]))
		start++;
	end = ft_strlen(s1);
	while (end > start && ft_strchr(set, s1[end - 1]))
		end--;
	trimmed = gc_malloc(ms, end - start + 1);
	if (!trimmed)
		return (NULL);
	ft_memcpy(trimmed, s1 + start, end - start);
	trimmed[end - start] = '\0';
	return (trimmed);
}

char	*gc_itoa(t_ms *ms, int n)
{
	char			buffer[12];
	int				i;
	int				neg;
	unsigned int	num;

	i = 10;
	buffer[11] = '\0';
	neg = (n < 0);
	if (neg)
		num = -n;
	else
		num = n;
	if (n == 0)
		return (gc_strdup(ms, "0"));
	while (num > 0 && i >= 0)
	{
		buffer[i--] = (num % 10) + '0';
		num /= 10;
	}
	if (neg)
		buffer[i--] = '-';
	return (gc_strdup(ms, &buffer[i + 1]));
}
