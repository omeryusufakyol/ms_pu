/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efsen <efsen@student.42kocaeli.com.tr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:10:46 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/09 21:01:44 by efsen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <unistd.h>
#include <stdlib.h>

int	has_plus_equal(const char *s)
{
	while (*s && *(s + 1))
	{
		if (*s == '+' && *(s + 1) == '=')
			return (1);
		s++;
	}
	return (0);
}

char	*ft_strjoin_free_both(t_ms *ms, char *s1, char *s2)
{
	if (!s1 && !s2)
		return (NULL);
	if (!s1)
		return (gc_strdup(ms, s2));
	if (!s2)
		return (gc_strdup(ms, s1));
	return (gc_strjoin(ms, s1, s2));
}

void	update_env_append(t_ms *ms, const char *arg)
{
	char	*key;
	char	*tmp;

	key = gc_strndup(ms, arg, ft_strnstr(arg, "+=", ft_strlen(arg)) - arg);
	tmp = gc_strjoin(ms, key, "=");
	update_env(ms, ft_strjoin_free_both(ms, tmp,
			gc_strjoin(ms, get_env_value(ms, key), ft_strchr(arg, '=') + 1)));
}

static int	is_valid_identifier(const char *s)
{
	int	i;

	if (!s || s[0] == '\0' || s[0] == '=' || ft_isdigit(s[0]))
		return (0);
	i = 0;
	while (s[i] && s[i] != '=')
	{
		if (s[i] == '+' && s[i + 1] == '=')
			break ;
		if (!(ft_isalnum(s[i]) || s[i] == '_'))
			return (0);
		i++;
	}
	return (1);
}

static char	*remove_quotes(t_ms *ms, const char *s)
{
	size_t	len;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	if ((s[0] == '\'' || s[0] == '\"') && s[len - 1] == s[0])
		return (gc_strndup(ms, s + 1, len - 2));
	return (gc_strdup(ms, s));
}

static char	*clean_export_arg(t_ms *ms, const char *arg)
{
	const char	*eq;
	char		*key_val;
	char		*value_cleaned;

	eq = ft_strchr(arg, '=');
	if (!eq)
		return (gc_strdup(ms, arg));
	key_val = gc_strndup(ms, arg, eq - arg);
	value_cleaned = remove_quotes(ms, eq + 1);
	return (gc_strjoin(ms, gc_strjoin(ms, key_val, "="), value_cleaned));
}

void	update_env(t_ms *ms, const char *arg)
{
	char	**new_env;
	char	*clean;
	int		i;
	int		len;

	clean = clean_export_arg(ms, arg);
	if (update_if_exists(ms, clean))
		return ;
	len = 0;
	while (ms->env && ms->env[len])
		len++;
	new_env = gc_malloc(ms, sizeof(char *) * (len + 2));
	if (!new_env)
		return ;
	i = -1;
	while (++i < len)
		new_env[i] = ms->env[i];
	new_env[i] = clean;
	new_env[i + 1] = NULL;
	ms->env = new_env;
}

static void	print_export_error(char *arg, t_ms *ms)
{
	write(2, "minishell: export: `", 21);
	write(2, arg, ft_strlen(arg));
	write(2, "': not a valid identifier\n", 27);
	ms->last_exit = 1;
}

int	ft_export(char **args, t_ms *ms)
{
	char	*key;
	int		i;

	if (!args[1])
		return (print_export(ms), 0);
	i = 1;
	while (args[i])
	{
		if (!is_valid_identifier(args[i]))
			return (print_export_error(args[i], ms), 1);
		if (has_plus_equal(args[i]))
			update_env_append(ms, args[i]);
		else if (has_equal(args[i]))
		{
			key = gc_strndup(ms, args[i], ft_strchr(args[i], '=') - args[i]);
			remove_from_export_only(ms, key);
			update_env(ms, args[i]);
		}
		else
			add_to_export_only(ms, args[i], 0);
		i++;
	}
	return (0);
}
