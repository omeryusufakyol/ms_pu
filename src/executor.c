/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 23:42:24 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:34:22 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
//     sadasdasdasd
static void	pre_exec_checks_and_builtins(t_cmd *cmds, t_ms *ms)
{
	int	ret;

	ret = 0;
	if (cmds->redirect_error)
	{
		write(2, "minishell: : No such file or directory\n", 39);
		run_single_cleanup_exit(ms, 1);
	}
	if (cmds->args && cmds->args[0] && cmds->args[0][0] == '\0'
		&& !cmds->infile && !cmds->outfile && !cmds->heredoc
		&& cmds->heredoc_fd < 0)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(": command not found\n", 2);
		run_single_cleanup_exit(ms, 127);
	}
	if (!cmds->args || !cmds->args[0])
		run_single_cleanup_exit(ms, 0);
	if (is_builtin(cmds->args[0]))
	{
		ret = run_builtin(cmds, ms);
		gc_free_all(ms);
		exit(ret);
	}
}

static void	exec_with_path_or_error(char *path, t_cmd *cmds, t_ms *ms)
{
	struct stat	sb;

	if (!path)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmds->args[0], 2);
		ft_putstr_fd(": command not found\n", 2);
		run_single_cleanup_exit(ms, 127);
	}
	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmds->args[0], 2);
		ft_putstr_fd(": Is a directory\n", 2);
		run_single_cleanup_exit(ms, 126);
	}
	execve(path, cmds->args, ms->env);
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmds->args[0], 2);
	ft_putstr_fd(": ", 2);
	perror("");
	if (errno == ENOENT)
		run_single_cleanup_exit(ms, 127);
	else
		run_single_cleanup_exit(ms, 126);
}

static void	run_single(t_cmd *cmds, t_ms *ms)
{
	pid_t	pid;
	int		status;
	char	*path;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	pid = fork();
	if (pid == 0)
	{
		signal(SIGPIPE, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		redirect(cmds, ms);
		pre_exec_checks_and_builtins(cmds, ms);
		path = find_path(ms, cmds->args[0], ms->env);
		exec_with_path_or_error(path, cmds, ms);
	}
	waitpid(pid, &status, 0);
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigquit);
	close_all_heredocs(cmds);
	if (WIFEXITED(status))
		ms->last_exit = WEXITSTATUS(status);
}

static int	handle_pipeline_or_builtin(t_cmd *cmds, t_ms *ms)
{
	if (cmds->next)
	{
		run_pipeline(cmds, ms);
		return (1);
	}
	if (is_builtin(cmds->args[0])
		&& !cmds->infile && !cmds->outfile && !cmds->heredoc)
	{
		ms->last_exit = run_builtin(cmds, ms);
		return (1);
	}
	return (0);
}

void	execute(t_cmd *cmds, t_ms *ms)
{
	if (!cmds)
		return ;
	if (!cmds->args || !*cmds->args)
	{
		if (cmds->heredoc || cmds->infile || cmds->outfile)
		{
			if (cmds->next)
			{
				run_pipeline(cmds, ms);
				return ;
			}
			run_single(cmds, ms);
		}
		return ;
	}
	if (handle_pipeline_or_builtin(cmds, ms))
		return ;
	run_single(cmds, ms);
}
