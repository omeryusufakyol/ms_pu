/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipeline_yedek.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 00:46:57 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/12 21:48:50 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

static int	wait_all(pid_t *pids, int count)
{
	int	i;
	int	status;
	int	last;

	i = 0;
	last = 0;
	while (i < count)
	{
		waitpid(pids[i], &status, 0);
		if (i == count - 1)
			last = status;
		i++;
	}
	if (WIFEXITED(last))
		return (WEXITSTATUS(last));
	if (WIFSIGNALED(last))
		return (128 + WTERMSIG(last));
	return (1);
}

static void	close_nonstd_under64(void)
{
	int	fd;

	fd = 3;
	while (fd < 64)
	{
		close(fd);
		fd++;
	}
}

static pid_t	launch_process(t_cmd *cmd, t_ms *ms, int in_fd, int out_fd)
{
	pid_t		pid;
	int			exit_code;
	char		*path;
	struct stat	sb;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	pid = fork();
	if (pid == 0)
	{
		signal(SIGPIPE, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		if (cmd->heredoc_fd >= 0)
		{
			dup2(cmd->heredoc_fd, STDIN_FILENO);
			close(cmd->heredoc_fd);
		}
		else if (in_fd != STDIN_FILENO)
			dup2(in_fd, STDIN_FILENO);
		if (out_fd != STDOUT_FILENO)
			dup2(out_fd, STDOUT_FILENO);
		if (in_fd != STDIN_FILENO)
			close(in_fd);
		if (out_fd != STDOUT_FILENO)
			close(out_fd);
		close_nonstd_under64();
		redirect(cmd, ms);
		if (cmd->redirect_error)
		{
			if (ms->last_exit)
			{
				gc_free_all(ms);
				exit(ms->last_exit);
			}
			gc_free_all(ms);
			exit(1);
		}
		if ((!cmd->args || !cmd->args[0])
			&& (cmd->infile || cmd->outfile
				|| cmd->heredoc || cmd->heredoc_fd >= 0))
		{
			gc_free_all(ms);
			exit(0);
		}
		if ((!cmd->args || !cmd->args[0])
			&& !cmd->outfile && !cmd->infile
			&& !cmd->heredoc && cmd->heredoc_fd < 0)
		{
			write(2, "minishell: invalid command\n", 28);
			gc_free_all(ms);
			exit(1);
		}
		if (is_builtin(cmd->args[0]))
		{
			exit_code = run_builtin(cmd, ms);
			gc_free_all(ms);
			exit(exit_code);
		}
		path = find_path(ms, cmd->args[0], ms->env);
		if (!path)
		{
			if (cmd->args[0] && is_redirect(cmd->args[0]))
			{
				gc_free_all(ms);
				exit(0);
			}
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(cmd->args[0], 2);
			ft_putstr_fd(": command not found\n", 2);
			gc_free_all(ms);
			exit(127);
		}
		if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(cmd->args[0], 2);
			ft_putstr_fd(": Is a directory\n", 2);
			run_single_cleanup_exit(ms, 126);
		}
		execve(path, cmd->args, ms->env);
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd->args[0], 2);
		ft_putstr_fd(": ", 2);
		perror("");
		gc_free_all(ms);
		if (errno == ENOENT)
			exit(127);
		else
			exit(126);
	}
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigquit);
	return (pid);
}

static int	run_pipeline_loop(t_cmd **cmds, t_ms *ms, int *in_fd, pid_t *pids)
{
	int	p[2];
	int	i;
	int	hfd;

	i = 0;
	while (*cmds && (*cmds)->next)
	{
		if (i >= MAX_CMDS)
		{
			write(2, "minishell: too many piped commands\n", 35);
			return (-1);
		}
		hfd = -1;
		if ((*cmds)->heredoc_delims)
		{
			hfd = prepare_heredoc_fd_sa(*cmds, ms);
			if (hfd == -1)
			{
				if (*in_fd != STDIN_FILENO)
					close(*in_fd);
				close_nonstd_under64();
				return (-1);
			}
			(*cmds)->heredoc_fd = hfd;
		}
		if (pipe(p) == -1)
		{
			perror("pipe");
			if (*in_fd != STDIN_FILENO)
				close(*in_fd);
			close_nonstd_under64();
			return (-1);
		}
		pids[i++] = launch_process(*cmds, ms, *in_fd, p[1]);
		if (hfd >= 0)
		{
			close(hfd);
			close_nonstd_under64();
			(*cmds)->heredoc_fd = -1;
		}
		close(p[1]);
		if (*in_fd != STDIN_FILENO)
		{
			close(*in_fd);
			//close_nonstd_under64();
		}
		*in_fd = p[0];
		*cmds = (*cmds)->next;
	}
	return (i);
}

static void	print_redirect_errors(t_cmd *head)
{
	t_cmd	*c;

	c = head;
	while (c)
	{
		if (c->redirect_error)
			write(2, "minishell: : No such file or directory\n", 39);
		c = c->next;
	}
}

int	run_pipeline(t_cmd *cmds, t_ms *ms)
{
	int		in_fd;
	pid_t	pids[MAX_CMDS];
	int		i;
	t_cmd	*head;
	int		hfd;

	head = cmds;
	ms->heredoc_index = 1;
	in_fd = STDIN_FILENO;
	i = run_pipeline_loop(&cmds, ms, &in_fd, pids);
	if (i == -1)
	{
		if (ms->last_exit)
			return (ms->last_exit);
		return (1);
	}
	hfd = -1;
	if (cmds->heredoc_delims)
	{
		hfd = prepare_heredoc_fd_sa(cmds, ms);
		if (hfd == -1)
		{
			if (in_fd != STDIN_FILENO)
				close(in_fd);
			if (ms->last_exit)
				return (ms->last_exit);
			return (1);
		}
		cmds->heredoc_fd = hfd;
	}
	pids[i++] = launch_process(cmds, ms, in_fd, STDOUT_FILENO);
	if (hfd >= 0)
	{
		close(hfd);
		cmds->heredoc_fd = -1;
	}
	if (in_fd != STDIN_FILENO)
		close(in_fd);
	ms->last_exit = wait_all(pids, i);
	close_nonstd_under64();
	print_redirect_errors(head);
	return (ms->last_exit);
}
