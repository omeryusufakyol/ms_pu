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

void	close_nonstd_under64(void)
{
	int	fd;

	fd = 3;
	while (fd < 64)
	{
		close(fd);
		fd++;
	}
}

static void	child_signals_and_fds(t_cmd *cmd, int in_fd, int out_fd)
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
}

/* --- redirect uygula; hata ise uygun kodla çık --- */
static void	child_do_redirects_or_exit(t_cmd *cmd, t_ms *ms)
{
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
}

/* --- argüman yokken özel durumları ele al --- */
static void	child_validate_args_or_exit(t_cmd *cmd, t_ms *ms)
{
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
}

/* --- builtin ise çalıştır ve çık, değilse devam et --- */
static void	child_try_builtin_or_continue(t_cmd *cmd, t_ms *ms)
{
	int	exit_code;

	if (cmd->args && cmd->args[0] && is_builtin(cmd->args[0]))
	{
		signal(SIGPIPE, SIG_IGN);
		exit_code = run_builtin(cmd, ms);
		gc_free_all(ms);
		exit(exit_code);
	}
}

/* --- path bul; yoksa not found veya redirect-token durumu --- */
static char	*child_resolve_path_or_exit(t_cmd *cmd, t_ms *ms)
{
	char	*path;

	path = find_path(ms, cmd->args[0], ms->env);
	if (path)
		return (path);
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
	return (NULL);
}

/* --- dizin mi kontrol et; execve yap; hata ise errno’ya göre çık --- */
static void	child_exec_or_error(char *path, t_cmd *cmd, t_ms *ms)
{
	struct stat	sb;

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

static void parent_ignore_job_signals(void)
{
	struct sigaction ign;

	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = 0;
	sigaction(SIGINT, &ign, NULL);
	sigaction(SIGQUIT, &ign, NULL);
}

/* bekleme bitince parent'ta Ctrl-C'yi line-editing için geri aç */
static void parent_restore_signals(void)
{
	struct sigaction sa;

	sa.sa_handler = handle_sigint; /* readline uyumlu handler'ın */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	/* bash gibi SIGQUIT'i genelde ignore ederiz */
	signal(SIGQUIT, SIG_IGN);
}

static pid_t	launch_process(t_cmd *cmd, t_ms *ms, int in_fd, int out_fd)
{
	pid_t	pid;
	char	*path;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	pid = fork();
	if (pid == 0)
	{
		child_signals_and_fds(cmd, in_fd, out_fd);
		child_do_redirects_or_exit(cmd, ms);
		child_validate_args_or_exit(cmd, ms);
		child_try_builtin_or_continue(cmd, ms);
		path = child_resolve_path_or_exit(cmd, ms);
		child_exec_or_error(path, cmd, ms);
	}
	return (pid);
}

/* pipe aç; hata olursa in_fd kapanır ve -1 döner */
static int	open_pipe_or_fail(int *in_fd, int p[2])
{
	if (pipe(p) == -1)
	{
		perror("pipe");
		if (*in_fd != STDIN_FILENO)
			close(*in_fd);
		close_nonstd_under64();
		return (-1);
	}
	return (0);
}

/* yazma ucunu kapat, eski in_fd’yi kapat, yeni in_fd’yi ayarla */
static void	rotate_pipe_fds(int *in_fd, int r, int w)
{
	close(w);
	if (*in_fd != STDIN_FILENO)
		close(*in_fd);
	*in_fd = r;
}

static void	close_cmd_heredoc_parent(t_cmd *cmd)
{
	if (cmd && cmd->heredoc_fd >= 0)
	{
		close(cmd->heredoc_fd);
		cmd->heredoc_fd = -1;
	}
}

static int	run_pipeline_loop(t_cmd **cmds, t_ms *ms, int *in_fd, pid_t *pids)
{
	int	p[2];
	int	i;

	i = 0;
	while (*cmds && (*cmds)->next)
	{
		if (i >= MAX_CMDS)
		{
			write(2, "minishell: too many piped commands\n", 35);
			return (-1);
		}
		if (open_pipe_or_fail(in_fd, p) < 0)
			return (-1);
		pids[i] = launch_process(*cmds, ms, *in_fd, p[1]);
		i = i + 1;
		close_cmd_heredoc_parent(*cmds);
		rotate_pipe_fds(in_fd, p[0], p[1]);
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

static int	wait_and_cleanup(t_ms *ms, t_cmd *head, pid_t *pids, int nproc)
{
	ms->last_exit = wait_all(pids, nproc);
	close_nonstd_under64();
	print_redirect_errors(head);
	return (ms->last_exit);
}

static int	collect_all_heredocs(t_cmd *head, t_ms *ms)
{
	struct sigaction	old_int;
	struct sigaction	old_quit;
	struct sigaction	ign;
	t_cmd				*c;
	int					hfd;

	if (!head)
		return (0);
	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	ign.sa_flags = 0;
	sigaction(SIGINT, &ign, &old_int);
	sigaction(SIGQUIT, &ign, &old_quit);
	c = head;
	while (c)
	{
		if (c->heredoc_delims)
		{
			hfd = prepare_heredoc_fd(c, ms); /* SA'lı versiyona gerek yok */
			if (hfd == -1)
			{
				sigaction(SIGINT, &old_int, NULL);
				sigaction(SIGQUIT, &old_quit, NULL);
				if (!ms->last_exit)
					ms->last_exit = 130;
				return (-1);
			}
			c->heredoc_fd = hfd;
		}
		c = c->next;
	}
	sigaction(SIGINT, &old_int, NULL);
	sigaction(SIGQUIT, &old_quit, NULL);
	return (0);
}

int	run_pipeline(t_cmd *cmds, t_ms *ms)
{
	int		in_fd;
	pid_t	pids[MAX_CMDS];
	int		i;
	t_cmd	*head;

	head = cmds;
	ms->heredoc_index = 1;

	/* Tüm heredoc’ları fork’tan ÖNCE topla */
	if (collect_all_heredocs(head, ms) < 0)
		return (ms->last_exit ? ms->last_exit : 1);
	parent_ignore_job_signals();
	in_fd = STDIN_FILENO;
	i = run_pipeline_loop(&cmds, ms, &in_fd, pids);
	if (i == -1)
	{
		parent_restore_signals();
		return (ms->last_exit ? ms->last_exit : 1);
	}

	/* Son komutu forkla */
	pids[i] = launch_process(cmds, ms, in_fd, STDOUT_FILENO);
	i = i + 1;

	/* parent’ta fd’leri kapat */
	if (in_fd != STDIN_FILENO)
		close(in_fd);
	close_cmd_heredoc_parent(cmds); /* son komutun heredoc'u da kapansın */
	ms->last_exit = wait_and_cleanup(ms, head, pids, i);
	parent_restore_signals();
	if (ms->last_exit == 130)
		write(1, "\n", 1);
	return (wait_and_cleanup(ms, head, pids, i));
}
