#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

typedef struct	s_sh
{
	char		**cmd;
	char		**env;
	int			idx;
	int			excode;
}				t_sh;

typedef enum e_error
{
	FATAL,
	EXE,
	CD_BAD_ARG,
	CD_CHANGE_DIR,
} t_error;

int	ft_strequ(char *s1, char *s2)
{
	if (strcmp(s1, s2) == 0)
		return (1);
	return (0);
}

int ft_strlen(char *s)
{
	int i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

void ft_putstr_fd(char *s, int fd)
{
	write(fd, s, ft_strlen(s));
}

int str_error(int error_code, char *arg)
{
	if (error_code == FATAL)
		ft_putstr_fd("error: fatal\n", 2);
	if (error_code == EXE)
	{
		ft_putstr_fd("error: cannot execute ", 2);
		ft_putstr_fd(arg, 2);
		ft_putstr_fd("\n", 2);
	}
	if (error_code == CD_BAD_ARG)
		ft_putstr_fd("error: cd: bad arguments\n", 2);
	if (error_code == CD_CHANGE_DIR)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(arg, 2);
		ft_putstr_fd("\n", 2);
	}
	return (EXIT_FAILURE);
}

char **make_arg(t_sh *sh)
{
	int cnt;
	int i;
	int idx;
	char **res;

	cnt = 0;
	idx = sh->idx;
	while (sh->cmd[sh->idx]
		&& !ft_strequ(sh->cmd[sh->idx], ";")
		&& !ft_strequ(sh->cmd[sh->idx], "|"))
	{
		cnt++;
		sh->idx++;
	}
	if (!(res = malloc(sizeof(char*) * (cnt + 1))))
		return (0);
	i = 0;
	while (i < cnt)
		res[i++] = sh->cmd[idx++];
	res[i] = 0;
	return (res); 
}

void exec_non_btin(t_sh *sh, char **arg)
{
	execve(arg[0], arg, sh->env);
	str_error(EXE, arg[0]);
	exit(EXIT_FAILURE);
}

void get_exit_code(t_sh *sh, int status)
{
	if (WIFEXITED(status))
		sh->excode = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		sh->excode = 128 + WTERMSIG(status);
}

void non_btin(t_sh *sh)
{
	int cpid;
	int status;
	char **arg;

	if (!(arg = make_arg(sh)))
		exit(str_error(FATAL, NULL));
	if ((cpid = fork()) == -1)
		exit(str_error(FATAL, NULL));
	if (cpid == 0)
		exec_non_btin(sh, arg);
	else
	{
		free(arg);
		wait(&status);
		get_exit_code(sh, status);
	}
}

int count_pipes(t_sh *sh)
{
	int i;
	int cnt;
	
	cnt = 0;
	i = sh->idx;
	while (sh->cmd[i] && !ft_strequ(sh->cmd[i], ";"))
	{
		if (ft_strequ(sh->cmd[i], "|"))
			cnt++;
		i++;
	}
	return (cnt);
}

void create_pipes(int pipes[], int cnt)
{
	int i;

	i = 0;
	while (i < cnt)
	{
		if (pipe(pipes + (i * 2)) == -1)
			exit(str_error(FATAL, NULL));
		i++;
	}
}

void close_all(int pipes[], int cnt)
{
	int i = 0;

	while (i < 2 * cnt)
		close(pipes[i++]);
}

void dup2_and_close_pipes(int pipes[], int i, int cnt)
{
	if (i < cnt)
		dup2(pipes[i * 2 + 1], 1);
	if (i > 0)
		dup2(pipes[(i - 1) * 2], 0);
	close_all(pipes, cnt);
}

void close_pipes_and_wait(t_sh *sh, int pipes[], int cnt, int cpids[])
{
	int status;
	int i;

	close_all(pipes, cnt);
	i = 0;
	while (i < cnt + 1)
	{
		waitpid(cpids[i], &status, 0);
		i++;
	}
	get_exit_code(sh, status);
}

void piping(t_sh *sh, int cnt)
{
	int pipes[cnt * 2];
	int cpids[cnt + 1];
	int i;
	char **arg;

	create_pipes(pipes, cnt);
	i = 0;
	while (i < (cnt + 1))
	{
		if (ft_strequ(sh->cmd[sh->idx], "|"))
			sh->idx++;		
		if (!(arg = make_arg(sh)))
			exit(str_error(FATAL, NULL));
		if ((cpids[i] = fork()) == 0)
		{
			dup2_and_close_pipes(pipes, i, cnt);
			exec_non_btin(sh, arg);
		}
		else if (cpids[i] == -1)
			exit(str_error(FATAL, NULL));
		free(arg);
		i++;
	}
	close_pipes_and_wait(sh, pipes, cnt, cpids);
}

void cd_btin(t_sh *sh)
{
	char **arg = make_arg(sh);
	int cnt;

	if (!arg)
		exit(str_error(FATAL, NULL));
	cnt = 0;
	while (arg[cnt])
		cnt++;
	if (cnt == 1 || cnt >= 3)
		sh->excode = str_error(CD_BAD_ARG, NULL);
	else if (chdir(arg[1]) == -1)
		sh->excode = str_error(CD_CHANGE_DIR, arg[1]);
	else
		sh->excode = 0;
	free(arg);
}

int main(int ac, char **cmd, char **env)
{
	t_sh sh;
	int cnt;

	(void)ac;
	sh.cmd = cmd;
	sh.env = env;
	sh.idx = 1;
	sh.excode = 0;
	while (cmd[sh.idx])
	{
		if (ft_strequ(cmd[sh.idx], ";") && sh.idx++)
			continue;
		if (ft_strequ(cmd[sh.idx], "cd"))
			cd_btin(&sh);
		else if ((cnt = count_pipes(&sh)))
			piping(&sh, cnt);
		else
			non_btin(&sh);
	}
	return (sh.excode);
}
