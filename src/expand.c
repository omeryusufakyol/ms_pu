/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:29:53 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/13 04:32:33 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <stdlib.h>

static char	*quote_if_redir_token(t_ms *ms, const char *value)
{
	char	*quoted;

	if (!ft_strcmp(value, "<") || !ft_strcmp(value, ">")
		|| !ft_strcmp(value, ">>") || !ft_strcmp(value, "<<")
		|| !ft_strcmp(value, "|"))
	{
		quoted = gc_malloc(ms, ft_strlen(value) + 3);
		if (!quoted)
			return (NULL);
		quoted[0] = '"';
		ft_memcpy(quoted + 1, value, ft_strlen(value));
		quoted[1 + ft_strlen(value)] = '"';
		quoted[2 + ft_strlen(value)] = '\0';
		return (quoted);
	}
	return (gc_strdup(ms, value));
}

char	*get_env_value(t_ms *ms, const char *name)
{
	int		i;
	size_t	len;
	char	*value;

	i = 0;
	len = ft_strlen(name);
	if (!name || name[0] == '\0')
		return (gc_strdup(ms, "$"));
	while (ms->env[i])
	{
		if (!ft_strncmp(ms->env[i], name, len) && ms->env[i][len] == '=')
		{
			value = &ms->env[i][len + 1];
			return (quote_if_redir_token(ms, value));
		}
		i++;
	}
	return (gc_strdup(ms, ""));
}

static char	*ft_strjoin_free(t_ms *ms, char *s1, char *s2)
{
	char	*joined;

	joined = gc_strjoin(ms, s1, s2);
	return (joined);
}

static int	check_tilde(const char *token, t_ms *ms, char **result)
{
	char	*home;
	char	*rest;

	if (!token || token[0] != '~')
		return (0);
	if (token[1] && token[1] != '/')
		return (0);
	home = get_env_value(ms, "HOME");
	if (!home)
		return (0);
	if (token[1] == '\0')
		*result = home;
	else
	{
		rest = gc_strdup(ms, token + 1);
		*result = ft_strjoin_free(ms, home, rest);
	}
	return (1);
}

static int	h_quote_toggle(const char *p, t_ms *ms, char **res, char *q)
{
	if (!*q && (*p == '\'' || *p == '"'))
	{
		*q = *p;
		*res = ft_strjoin_free(ms, *res, gc_strndup(ms, p, 1));
		return (1);
	}
	if (*q && *p == *q)
	{
		*res = ft_strjoin_free(ms, *res, gc_strndup(ms, p, 1));
		*q = 0;
		return (1);
	}
	return (0);
}

static int	h_single_quote_block(const char *p, t_ms *ms, char **res)
{
	size_t	j;
	char	*tmp;

	j = 0;
	while (p[j] && p[j] != '\'')
		j++;
	if (j)
	{
		tmp = gc_strndup(ms, p, j);
		*res = ft_strjoin_free(ms, *res, tmp);
	}
	return ((int)j);
}

static int	h_dollar_var(const char *p, t_ms *ms, char **res)
{
	size_t	j;
	char	*tmp;

	j = 2;
	while (p[j] && (ft_isalnum(p[j]) || p[j] == '_'))
		j++;
	tmp = gc_strndup(ms, p + 1, j - 1);
	tmp = get_env_value(ms, tmp);
	*res = ft_strjoin_free(ms, *res, tmp);
	return ((int)j);
}

static int	h_dollar(const char *p, t_ms *ms, char **res, char *q)
{
	char	*tmp;

	if (*p != '$')
		return (0);
	if (!*q && (p[1] == '\'' || p[1] == '"'))
	{
		*q = p[1];
		*res = ft_strjoin_free(ms, *res, gc_strndup(ms, &p[1], 1));
		return (2);
	}
	if (p[1] == '?')
	{
		tmp = gc_itoa(ms, ms->last_exit);
		*res = ft_strjoin_free(ms, *res, tmp);
		return (2);
	}
	if (p[1] && (ft_isalnum(p[1]) || p[1] == '_'))
		return (h_dollar_var(p, ms, res));
	*res = ft_strjoin_free(ms, *res, gc_strdup(ms, "$"));
	return (1);
}

static int	h_plain(const char *p, t_ms *ms, char **res, char q)
{
	size_t	j;
	char	*tmp;

	j = 0;
	if (q == '"')
	{
		while (p[j] && p[j] != '"' && p[j] != '$')
			j++;
	}
	else
	{
		while (p[j] && p[j] != '$' && p[j] != '\'' && p[j] != '"')
			j++;
	}
	if (j)
	{
		tmp = gc_strndup(ms, p, j);
		*res = ft_strjoin_free(ms, *res, tmp);
	}
	return ((int)j);
}

static int	h_step(const char *p, t_ms *ms, char **res, char *q)
{
	int	c;

	c = h_quote_toggle(p, ms, res, q);
	if (c)
		return (c);
	if (*q == '\'')
	{
		c = h_single_quote_block(p, ms, res);
		if (c)
			return (c);
	}
	c = h_dollar(p, ms, res, q);
	if (c)
		return (c);
	c = h_plain(p, ms, res, *q);
	return (c);
}

static char	*expand_token(const char *token, t_ms *ms)
{
	char	*result;
	size_t	i;
	char	q;
	int		c;

	if (!token)
		return (NULL);
	if (check_tilde(token, ms, &result))
		return (result);
	result = gc_strdup(ms, "");
	i = 0;
	q = 0;
	while (token[i])
	{
		c = h_step(token + i, ms, &result, &q);
		if (c)
			i += c;
		else
			i++;
	}
	return (result);
}

static char	*quote_slice(t_ms *ms, const char *s, size_t len)
{
	return (ft_strjoin_free(ms, gc_strdup(ms, "\""),
			ft_strjoin_free(ms, gc_strndup(ms, s, len), gc_strdup(ms, "\""))));
}

static char	*quote_cstr(t_ms *ms, const char *s)
{
	return (ft_strjoin_free(ms, gc_strdup(ms, "\""),
			ft_strjoin_free(ms, gc_strdup(ms, s), gc_strdup(ms, "\""))));
}

static size_t	first_ws(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i] && s[i] != ' ' && s[i] != '\t')
		i++;
	return (i);
}

static size_t	skip_ws(const char *s, size_t j)
{
	while (s[j] == ' ' || s[j] == '\t')
		j++;
	return (j);
}

static int	append_split_if_needed(char *token, t_ms *ms, char **res)
{
	char	*expanded;
	size_t	i;
	size_t	j;

	expanded = expand_token(token, ms);
	if (expanded[0] == '\0' && token[0] != '"' && token[0] != '\'')
		return (0);
	if (token[0] == '"' || token[0] == '\'')
	{
		res[0] = expanded;
		return (1);
	}
	i = first_ws(expanded);
	if (expanded[i] == '\0')
	{
		res[0] = expanded;
		return (1);
	}
	res[0] = quote_slice(ms, expanded, i);
	j = skip_ws(expanded, i);
	if (expanded[j] == '\0')
		return (1);
	res[1] = quote_cstr(ms, expanded + j);
	return (2);
}

static int	append_as_is(char *token, t_ms *ms, char **res)
{
	char	*expanded;

	expanded = expand_token(token, ms);
	if (expanded[0] == '\0' && token[0] != '"' && token[0] != '\'')
		return (0);
	res[0] = expanded;
	return (1);
}

static char	**resize_result(t_ms *ms, char **old, int old_cap, int new_cap)
{
	char	**new;
	int		i;

	new = gc_malloc(ms, sizeof(char *) * new_cap);
	if (!new)
		return (NULL);
	i = 0;
	while (i < old_cap)
	{
		new[i] = old[i];
		i++;
	}
	return (new);
}

static int	count_split_pieces(char *token, t_ms *ms)
{
	char	*expanded;
	size_t	i;
	size_t	j;

	expanded = expand_token(token, ms);
	if (!expanded)
		return (0);
	if (expanded[0] == '\0' && token[0] != '"' && token[0] != '\'')
		return (0);
	if (token[0] == '"' || token[0] == '\'')
		return (1);
	i = 0;
	while (expanded[i] && expanded[i] != ' ' && expanded[i] != '\t')
		i++;
	if (expanded[i] == '\0')
		return (1);
	j = i;
	while (expanded[j] == ' ' || expanded[j] == '\t')
		j++;
	if (expanded[j] == '\0')
		return (1);
	return (2);
}

static int	needs_split(int index)
{
	return (index == 0);
}

static char	**ensure_capacity(t_ms *ms, char **dst, int *cap, int need)
{
	char	**tmp;

	while (need >= *cap)
	{
		tmp = resize_result(ms, dst, *cap, (*cap) * 2);
		if (!tmp)
			return (NULL);
		dst = tmp;
		*cap = (*cap) * 2;
	}
	return (dst);
}

static int	pieces_for_index(int i, char **tokens, t_ms *ms)
{
	int		split;

	split = needs_split(i);
	if (split)
		return (count_split_pieces(tokens[i], ms));
	return (1);
}

static int	append_token_with_index(int i, char **tokens, t_ms *ms, char **slot)
{
	int		split;

	split = needs_split(i);
	if (split)
		return (append_split_if_needed(tokens[i], ms, slot));
	return (append_as_is(tokens[i], ms, slot));
}

/* --- 25 satırı geçmeyen ana fonksiyon --- */
char	**expand_tokens(char **tokens, t_ms *ms)
{
	char	**result;
	int		i;
	int		j;
	int		capacity;
	int		need;

	capacity = 8;
	result = gc_malloc(ms, sizeof(char *) * capacity);
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (tokens[i])
	{
		need = j + pieces_for_index(i, tokens, ms);
		result = ensure_capacity(ms, result, &capacity, need);
		if (!result)
			return (NULL);
		j = j + append_token_with_index(i, tokens, ms, &result[j]);
		i = i + 1;
	}
	result[j] = NULL;
	return (result);
}

// BUNA DIKKAT