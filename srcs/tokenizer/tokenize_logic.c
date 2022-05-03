#include "minishell.h"

static int	tokenize__logic_pipe(t_tokenizer *tknzer)
{
	tknzer->pipe = 1;
	tknzer->stop_parse_str = 1;
	if (pipe(tknzer->fd_pipe) == -1)
	{
		ms_print_cmd_error("pipe()", strerror(errno));
		return (4);
	}
	tknzer->fd_edited[STDOUT_FILENO] = 1;
	tknzer->cmd_now->fd[STDOUT_FILENO] = tknzer->fd_pipe[STDOUT_FILENO];
	return (0);
}

static int	tokenize__logic_str(t_dlst **tokens, t_dlst *lexemes,
								t_tokenizer *tknzer)
{
	if (tknzer->cmd_now == NULL || tknzer->stop_parse_str == 1)
	{
		create_cmd(tknzer, tokens,
			ft_strdup(((t_lexeme *)lexemes->content)->str));
	}
	else
	{
		tknzer->cmd_now->cmd
			= (char **)arr_add_back((void **)tknzer->cmd_now->cmd,
				ft_strdup(((t_lexeme *)lexemes->content)->str));
	}
	return (0);
}

static int	tokenize__logic_heredoc(t_dlst *lexemes, t_tokenizer *tknzer)
{
	pid_t	pid;
	int		fd_pipe[2];

	tknzer->stop_parse_str = 1;
	if (pipe(fd_pipe) == -1)
	{
		ms_print_cmd_error("pipe()", strerror(errno));
		return (errno);
	}
	pid = fork();
	if (pid < 0)
	{
		ms_print_cmd_error("fork()", strerror(errno));
		return (errno);
	}
	else if (pid == 0)
		exit(heredoc(fd_pipe[STDOUT_FILENO], lexemes));
	pid = wait_process(pid);
	close(fd_pipe[STDOUT_FILENO]);
	tknzer->cmd_now->fd[STDIN_FILENO] = fd_pipe[STDIN_FILENO];
	if (pid != 0)
	{
		ms_print_cmd_error("process error", strerror(pid));
		return (errno);
	}
	return (-1);
}

int	tokenize__logic(t_dlst **tokens, t_dlst *lexemes, t_tokenizer *tknzer)
{
	t_operator	lexeme_type;

	lexeme_type = ((t_lexeme *)lexemes->content)->type;
	if (lexeme_type == L_REDIR)
		return (redirect(STDIN_FILENO, O_RDONLY, lexemes, tknzer));
	else if (lexeme_type == R_REDIR)
	{
		return (redirect(STDOUT_FILENO,
				O_CREAT | O_WRONLY | O_TRUNC, lexemes, tknzer));
	}
	else if (lexeme_type == R_HEREDOC)
	{
		return (redirect(STDOUT_FILENO,
				O_CREAT | O_WRONLY | O_APPEND, lexemes, tknzer));
	}
	else if (lexeme_type == L_HEREDOC)
		return (tokenize__logic_heredoc(lexemes, tknzer));
	else if (lexeme_type == STR)
		return (tokenize__logic_str(tokens, lexemes, tknzer));
	else if (lexeme_type == PIPE)
		return (tokenize__logic_pipe(tknzer));
	return (3);
}