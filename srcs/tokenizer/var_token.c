#include "minishell.h"

t_list_var	loop_vars__iterator(t_dlst *tokens, t_dlst *prev)
{
	t_dlst		*temp;
	t_list_var	list;

	list.var2 = prev;
	if (((t_token *)tokens->content)->type == STR
		&& ((t_token *)tokens->content)->str[0] == '\0')
	{
		temp = tokens->next;
		free_token(tokens->content);
		dlst_remove_node(tokens);
		list.var1 = temp;
	}
	else
	{
		list.var2 = tokens;
		list.var1 = tokens->next;
	}
	return (list);
}

t_dlst	*loop_vars(t_dlst *tokens)
{
	t_dlst		*prev;
	t_list_var	list;

	prev = NULL;
	while (tokens)
	{
		if (((t_token *)tokens->content)->type == SET_VAR)
		{
			tokens = parse__var(tokens->content, tokens);
			prev = tokens;
		}
		else
		{
			list = loop_vars__iterator(tokens, prev);
			tokens = list.var1;
			prev = list.var2;
		}
	}
	tokens = dlst_last_node(prev);
	return (tokens);
}

char	*join_var_str(char *str, int end_var, char *res)
{
	char	*value;
	char	end_symb;

	end_symb = str[end_var];
	str[end_var] = '\0';
	value = get_env(str);
	if (value)
		res = kl_strjoin_free(res, value);
	str[end_var] = end_symb;
	return (res);
}

void	join_var__iterator(char *str, t_join_var *s_jv)
{
	char	*temp;

	if (s_jv->end_prev_var != s_jv->start_var)
	{
		str[s_jv->start_var] = '\0';
		temp = s_jv->res;
		s_jv->res = ft_strjoin(s_jv->res, str + s_jv->end_prev_var);
		free(temp);
		str[s_jv->start_var] = VAR;
	}
	s_jv->end_var = s_jv->start_var + 1;
	while (str[s_jv->end_var] && str[s_jv->end_var] != VAR
		&& str[s_jv->end_var] != SPACE)
		++s_jv->end_var;
	s_jv->res
		= join_var_str(str + s_jv->start_var + 1,
			s_jv->end_var - s_jv->start_var - 1,
			s_jv->res);
	s_jv->end_prev_var = s_jv->end_var;
	s_jv->start_var = ft_strichr(str + s_jv->end_var, VAR);
	if (s_jv->start_var != -1)
		s_jv->start_var += s_jv->end_var;
}

t_token	*join_var(char *str, int start_var)
{
	char		*temp;
	t_join_var	*s_jv;

	s_jv = kl_malloc(sizeof(t_join_var));
	ft_bzero(s_jv, sizeof(t_join_var));
	s_jv->res = ft_strdup("");
	s_jv->start_var = start_var;
	while (s_jv->start_var >= 0)
		join_var__iterator(str, s_jv);
	if (str[s_jv->end_var])
	{
		temp = s_jv->res;
		s_jv->res = ft_strjoin(s_jv->res, str + s_jv->end_var);
		free(temp);
	}
	free(str);
	return (new_token(STR, s_jv->res));
}