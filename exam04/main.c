#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct s_sh
{
	char **cmd;
	char **env;
	int idx;
	int excode;
} t_sh;

enum e_error
{
	FATAL,
	EXE,
	BAD_ARG,
	BAD_DIR,
};

int ft_strequ(char *s1, char *s2)
{
	if (strcmp(s1, s2) == 0)
		return (1);
	return (0);
}

int ft_strlen(char *s)
{
	int i = 0;
	while (s[i])
		i++;
	return (i);
}

void ft_eprint(char *s)
{
	write(2, s, ft_strlen(s));
}

int errmsg(int err, char *arg)
{
	if (err == FATAL)
		ft_eprint("error: fatal\n");
	if (err == EXE)
	{
		ft_eprint("error: cannot execute ");
		ft_eprint(arg);
		ft_eprint("\n");
	}
	if (err == BAD_ARG)
		ft_eprint("error: cd: bad arguments\n");
	if (err == BAD_DIR)
	{
		ft_eprint("error: cd: cannot change directory to ");
		ft_eprint(arg);
		ft_eprint("\n");
	}
	return (EXIT_FAILURE);
}

char **make_arg(t_sh *sh)
{
	int cnt = 0;
	int idx = sh->idx;

	while (sh->cmd[sh->idx]
		&& !ft_strequ(sh->cmd[sh->idx], ";")
		&& !ft_strequ(sh->cmd[sh->idx], "|"))
	{
		cnt++;
		sh->idx++;
	}
	
	char **res = malloc(sizeof(char*) * (cnt + 1));
	if (!res)
		exit(errmsg(FATAL, NULL));
	int i = 0;
	while (i < cnt)
		res[i++] = sh->cmd[idx++];
	res[i] = 0;
	return (res);
}

void btin(t_sh *sh)
{
	char **arg = make_arg(sh);
	int cnt = 0;

	while (arg[cnt])
		cnt++;
	if (cnt != 2)
		sh->excode = errmsg(BAD_ARG, NULL);
	else if (chdir(arg[1]) == -1)
		sh->excode = errmsg(BAD_DIR, arg[1]);
	else
		sh->excode = 0;
	free(arg);
}

int count_pipes(t_sh *sh)
{
	int cnt = 0;
	int idx = sh->idx;
	while (sh->cmd[idx]
		&& !ft_strequ(sh->cmd[idx], ";"))
	{
		if (ft_strequ(sh->cmd[idx], "|"))
			cnt++;
		idx++;
	}
	return (cnt);
}

void create_pipes(int pipes[], int nb_pipe)
{
	int i = 0;
	while (i < nb_pipe)
	{
		if (pipe(pipes + (i * 2)) < 0)
			exit(errmsg(FATAL, NULL));
		i++;
	}
}

void close_all(int pipes[], int nb_pipe)
{
	int i = 0;
	while (i < 2 * nb_pipe)
		close(pipes[i++]);
}

void get_exit_code(t_sh *sh, int status)
{
	if (WIFEXITED(status))
		sh->excode = WEXITSTATUS(status);
}

void wait_and_get_exit_code(t_sh *sh, int cpids[], int nb_pipe)
{
	int status;
	int i = 0;
	while (i < nb_pipe + 1)
	{
		waitpid(cpids[i], &status, 0);
		i++;
	}
	get_exit_code(sh, status);
}

void piping(t_sh *sh, int nb_pipe)
{
	int pipes[nb_pipe * 2];
	int cpids[nb_pipe + 1];

	create_pipes(pipes, nb_pipe);
	int i = 0;
	char **arg;
	while (i < nb_pipe + 1)
	{
		if (ft_strequ(sh->cmd[sh->idx], "|"))
			sh->idx++;
		arg = make_arg(sh);
		cpids[i] = fork();
		if (cpids[i] < 0)
			exit(errmsg(FATAL, NULL));
		if (cpids[i] == 0)
		{
			if (i < nb_pipe)
				dup2(pipes[i * 2 + 1], 1);
			if (i > 0)
				dup2(pipes[(i - 1) * 2], 0);
			close_all(pipes, nb_pipe);
			execve(arg[0], arg, sh->env);
			exit(errmsg(EXE, arg[0]));
		}
		free(arg);
		i++;
	}
	close_all(pipes, nb_pipe);
	wait_and_get_exit_code(sh, cpids, nb_pipe);
}

void non_btin(t_sh *sh)
{
	int cpid;
	int status;
	char **arg = make_arg(sh);

	cpid = fork();
	if (cpid < 0)
		exit(errmsg(FATAL, NULL));
	if (cpid == 0)
	{
		execve(arg[0], arg, sh->env);
		exit(errmsg(EXE, arg[0]));
	}
	else
	{
		free(arg);
		wait(&status);
		get_exit_code(sh, status);
	}
}

int main(int ac, char **av, char **env)
{
	(void)ac;

	t_sh sh;
	sh.cmd = av;
	sh.env = env;
	sh.idx = 1;
	sh.excode = 0;
	int nb_pipe;

	while (sh.cmd[sh.idx])
	{
		if (ft_strequ(sh.cmd[sh.idx], ";"))
			sh.idx++;
		else if (ft_strequ(sh.cmd[sh.idx], "cd"))
			btin(&sh);
		else if ((nb_pipe = count_pipes(&sh)))
			piping(&sh, nb_pipe);
		else
			non_btin(&sh);
	}
	return (sh.excode);
}