/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_utils2.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 15:33:53 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/18 15:34:14 by oakyol           ###   ########.fr       */
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

static void	handle_outfile(t_cmd *cmd, t_ms *ms)
{
	int	fd;

	if (cmd->append)
		fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		perror(cmd->outfile);
		gc_free_all(ms);
		close_nonstd_under64();
		exit(1);
	}
	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		perror("dup2");
		close(fd);
		gc_free_all(ms);
		close_nonstd_under64();
		exit(1);
	}
	close(fd);
}

static void	redirect_heredoc_if_needed(t_cmd *cmd, t_ms *ms)
{
	struct sigaction	old_int;
	struct sigaction	old_quit;
	struct sigaction	ign;

	if (!(cmd->heredoc && cmd->heredoc_fd < 0))
		return ;
	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = 0;
	sigaction(SIGINT, &ign, &old_int);
	sigaction(SIGQUIT, &ign, &old_quit);
	if (handle_heredoc(cmd, ms))
	{
		gc_free_all(ms);
		exit(ms->last_exit);
	}
	sigaction(SIGINT, &old_int, NULL);
	sigaction(SIGQUIT, &old_quit, NULL);
}

static void	redirect_helper(t_cmd *cmd, t_ms *ms)
{
	int	fd;

	if (cmd->def_out_pending && cmd->def_out_path)
	{
		if (cmd->def_out_append)
			fd = open(cmd->def_out_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = open(cmd->def_out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
		{
			perror(cmd->def_out_path);
			gc_free_all(ms);
			close_nonstd_under64();
			exit(1);
		}
		close(fd);
		cmd->def_out_pending = 0;
	}
}

void	redirect(t_cmd *cmd, t_ms *ms)
{
	int					fd;

	redirect_heredoc_if_needed(cmd, ms);
	redirect_helper(cmd, ms);
	if (cmd->infile)
	{
		fd = open(cmd->infile, O_RDONLY);
		if (fd < 0)
		{
			perror(cmd->infile);
			gc_free_all(ms);
			close_nonstd_under64();
			exit(1);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
	else if (cmd->heredoc_fd >= 0)
	{
		dup2(cmd->heredoc_fd, STDIN_FILENO);
		close(cmd->heredoc_fd);
		cmd->heredoc_fd = -1;
	}
	if (cmd->outfile)
		handle_outfile(cmd, ms);
}

void	close_all_heredocs(t_cmd *cmd)
{
	while (cmd)
	{
		if (cmd->heredoc_fd >= 0)
		{
			close(cmd->heredoc_fd);
			cmd->heredoc_fd = -1;
		}
		cmd = cmd->next;
	}
}
