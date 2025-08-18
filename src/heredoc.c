/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:29:53 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/15 04:11:10 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <stdlib.h>
#include <stdio.h>

volatile sig_atomic_t	g_heredoc_sigint = 0;

void	add_heredoc(t_cmd *cmd, char *delim, t_ms *ms)
{
	int		i;
	int		j;
	char	**new;

	i = 0;
	while (cmd->heredoc_delims && cmd->heredoc_delims[i])
		i++;
	new = gc_malloc(ms, sizeof(char *) * (i + 2));
	if (!new)
		return ;
	j = 0;
	while (j < i)
	{
		new[j] = cmd->heredoc_delims[j];
		j++;
	}
	new[i] = gc_strdup(ms, delim);
	new[i + 1] = NULL;
	cmd->heredoc_delims = new;
}

static int	check_delim_quoted(const char *input, int i)
{
	i += 2;
	while (input[i] == ' ' || input[i] == '\t')
		i++;
	while (input[i] && input[i] != ' ' && input[i] != '\t'
		&& input[i] != '<' && input[i] != '>' && input[i] != '|')
	{
		if (input[i] == '\'' || input[i] == '"')
			return (1);
		i++;
	}
	return (0);
}

int	is_quoted_delim(t_ms *ms)
{
	char	*input;
	int		i;
	int		index;

	i = 0;
	index = 1;
	input = ms->raw_input;
	while (input[i])
	{
		if (input[i] == '<' && input[i + 1] == '<')
		{
			if (index == ms->heredoc_index)
				return (check_delim_quoted(input, i));
			index++;
			i += 2;
		}
		else
			i++;
	}
	return (0);
}

static void	read_heredoc_loop(int write_fd, char *delim, t_ms *ms)
{
	char	*line;
	char	*expanded;
	int		quoted;

	quoted = is_quoted_delim(ms);
	while (!g_heredoc_sigint)
	{
		line = readline("heredoc> ");
		if (!line)
			break ;
		if (!ft_strcmp(line, delim))
		{
			free(line);
			break ;
		}
		if (quoted)
			expanded = gc_strdup(ms, line);
		else
			expanded = expand_heredoc_line_envonly(line, ms);
		write(write_fd, expanded, ft_strlen(expanded));
		write(write_fd, "\n", 1);
		free(line);
	}
}

static t_ms	*ms_holder(t_ms *new_ms)
{
	static t_ms	*saved = NULL;

	if (new_ms)
		saved = new_ms;
	return (saved);
}

static void	handle_heredoc_sigint(int sig)
{
	(void)sig;
	g_heredoc_sigint = 1;
	write(STDOUT_FILENO, "\n", 1);
	close(STDIN_FILENO);
}

static void	heredoc_child(int rfd, int wfd, char *delim, t_ms *ms)
{
	g_heredoc_sigint = 0;
	ms_holder(ms);
	signal(SIGINT, handle_heredoc_sigint);
	signal(SIGPIPE, SIG_DFL);
	close(rfd);
	read_heredoc_loop(wfd, delim, ms);
	close(wfd);
	if (g_heredoc_sigint)
		ms->last_exit = 130;
	gc_free_all(ms_holder(NULL));
	close_nonstd_under64();
	exit(ms->last_exit);
}

static int	heredoc_interrupted(int rfd, int status, t_ms *ms)
{
	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 130))
	{
		close(rfd);
		ms->last_exit = 130;
		return (1);
	}
	return (0);
}

static int	handle_one_heredoc(t_cmd *cmd, t_ms *ms, int i)
{
	int		fd[2];
	pid_t	pid;
	int		status;

	if (pipe(fd) == -1)
		return (perror("heredoc error"), 1);
	pid = fork();
	if (pid == -1)
		return (perror("heredoc error"), close(fd[0]), close(fd[1]), 1);
	if (pid == 0)
		heredoc_child(fd[0], fd[1], cmd->heredoc_delims[i], ms);
	close(fd[1]);
	waitpid(pid, &status, 0);
	if (heredoc_interrupted(fd[0], status, ms))
		return (1);
	if (!cmd->heredoc_delims[i + 1])
		cmd->heredoc_fd = fd[0];
	else
		close(fd[0]);
	ms->heredoc_index++;
	return (0);
}

int	handle_heredoc(t_cmd *cmd, t_ms *ms)
{
	int	i;

	i = 0;
	while (cmd->heredoc_delims && cmd->heredoc_delims[i])
	{
		if (handle_one_heredoc(cmd, ms, i))
			return (1);
		i++;
	}
	return (0);
}

static void	heredoc_child_fd(int rfd, int wfd, char *delim, t_ms *ms)
{
	g_heredoc_sigint = 0;
	ms_holder(ms);
	signal(SIGINT, handle_heredoc_sigint);
	signal(SIGPIPE, SIG_DFL);
	close(rfd);
	read_heredoc_loop(wfd, delim, ms);
	close(wfd);
	if (g_heredoc_sigint)
		ms->last_exit = 130;
	gc_free_all(ms_holder(NULL));
	close_nonstd_under64();
	exit(ms->last_exit);
}

static int	heredoc_setup_pipe_fork(int fd[2], int *plast_fd, pid_t *ppid)
{
	if (pipe(fd) == -1)
	{
		if (*plast_fd >= 0)
			close(*plast_fd);
		return (perror("heredoc error"), -1);
	}
	*ppid = fork();
	if (*ppid == -1)
	{
		if (*plast_fd >= 0)
			close(*plast_fd);
		close(fd[0]);
		close(fd[1]);
		return (perror("heredoc error"), -1);
	}
	return (0);
}

static int	heredoc_wait_and_check(int rfd, pid_t pid, t_ms *ms, int *plast_fd)
{
	int	status;

	waitpid(pid, &status, 0);
	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 130))
	{
		close(rfd);
		if (*plast_fd >= 0)
			close(*plast_fd);
		ms->last_exit = 130;
		return (1);
	}
	return (0);
}

int	prepare_heredoc_fd_sa(t_cmd *cmd, t_ms *ms)
{
	struct sigaction	old_int;
	struct sigaction	old_quit;
	struct sigaction	ign;
	int					fd;

	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = 0;
	sigaction(SIGINT, &ign, &old_int);
	sigaction(SIGQUIT, &ign, &old_quit);
	fd = prepare_heredoc_fd(cmd, ms);
	sigaction(SIGINT, &old_int, NULL);
	sigaction(SIGQUIT, &old_quit, NULL);
	return (fd);
}

int	prepare_heredoc_fd(t_cmd *cmd, t_ms *ms)
{
	int		fd[2];
	int		i;
	pid_t	pid;
	int		last_fd;

	i = 0;
	last_fd = -1;
	while (cmd->heredoc_delims && cmd->heredoc_delims[i])
	{
		if (heredoc_setup_pipe_fork(fd, &last_fd, &pid) == -1)
			return (-1);
		if (pid == 0)
			heredoc_child_fd(fd[0], fd[1], cmd->heredoc_delims[i], ms);
		close(fd[1]);
		if (heredoc_wait_and_check(fd[0], pid, ms, &last_fd))
			return (-1);
		if (cmd->heredoc_delims[i + 1])
			close(fd[0]);
		else
			last_fd = fd[0];
		ms->heredoc_index++;
		i++;
	}
	return (last_fd);
}
