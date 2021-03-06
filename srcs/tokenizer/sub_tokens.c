#include "minishell.h"

void	free_cmd(void *p)
{
	t_cmd	*s_cmd;

	s_cmd = p;
	if (s_cmd->fd[STDIN_FILENO] != STDIN_FILENO)
		close(s_cmd->fd[STDIN_FILENO]);
	if (s_cmd->fd[STDOUT_FILENO] != STDOUT_FILENO)
		close(s_cmd->fd[STDOUT_FILENO]);
	if (s_cmd->exec_file)
		free(s_cmd->exec_file);
	if (s_cmd->cmd)
		kl_free_arr(s_cmd->cmd);
	free(s_cmd);
}

int create_fd(char *path, int flags)
{
	int     fd_new;
	char	*error;

	fd_new = open(path, flags, 0644);
	if (fd_new == -1)
	{
		if (errno == ENOENT)
		{
			error = kl_strjoin_free(
				ft_strdup("no such file or directory: "),
				ft_strdup(path)
			);
			ms_print(STDERR_FILENO, COLOR_RED, error);
			free(error);
		}
		else
			ms_print_cmd_error("open()", strerror(errno));
		return (0);
	}
	return (fd_new);
}

int	add_fd(t_cmd *s_cmd, int fd, char *path, int flags)
{
	int fd_new;

	fd_new = create_fd(path, flags);
	if (fd_new == 0)
		return (errno);
	if (s_cmd->fd[fd] != fd)
		close(s_cmd->fd[fd]);
	s_cmd->fd[fd] = fd_new;
	return (0);
}

char	*get_str_lexeme(t_dlst *lexemes)
{
	t_operator	lexeme_type;
	char		*error;

	lexemes = lexemes->prev;
	if (lexemes == NULL)
		return (NULL);
	lexeme_type = ((t_lexeme *)lexemes->content)->type;
	if (lexeme_type == STR)
		return (((t_lexeme *)lexemes->content)->str);
	error = ft_strjoin(
		"syntax error near unexpected token ",
		(char *)&lexeme_type
	);
	ms_print(STDERR_FILENO, COLOR_RED, error);
	free(error);
	return (NULL);
}

void	multiline_put_in_file(void print_tag(void), char *delimiter, int fd)
{
	char	**line;

	line = kl_malloc(sizeof(char **));
	print_tag();
	while (get_next_line(STDIN_FILENO, line) > 0)
	{
		if (**line != '\0' && kl_strcmp(*line, delimiter) == 0)
			break ;
		ft_putstr_fd(*line, fd);
		ft_putchar_fd('\n', fd);
		free(*line);
		print_tag();
	}
	if (*line)
		free(*line);
	free(line);
}

void	free_tokenizer(t_dlst **tokens, t_dlst *lexemes, t_tokenizer *tknzer)
{
	lexemes = dlst_last_node(lexemes);
	dlst_loop(lexemes);
	dlst_map(lexemes, free_lexeme);
	dlst_free(lexemes);
	*tokens = dlst_last_node(*tokens);
	if (*tokens)
	{
		dlst_loop(*tokens);
		dlst_map(*tokens, free_cmd);
		dlst_free(*tokens);
		*tokens = NULL;
	}
	if (tknzer->cmd_now)
	{
		free(tknzer->cmd_now);
		tknzer->cmd_now = NULL;
	}
}
