#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

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
		return 127; // command not found ..?
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

void create_pipe(int pipes[], int nb_pipe, int i)
{
	if (i > nb_pipe)
		return;
	if (pipe(pipes + (i * 2)) < 0)
		exit(errmsg(FATAL, NULL));
}

void close_pipe(int pipes[], int nb_pipe, int i)
{
	if (i < nb_pipe)
		close(pipes[i * 2 + 1]);
	if (i > 0)
		close(pipes[(i - 1) * 2]);
}

void get_excode(t_sh *sh, int status)
{
	if (WIFEXITED(status))
		sh->excode = WEXITSTATUS(status);
}

void wait_and_get_excode(t_sh *sh, int cpid)
{
	int status;
	waitpid(cpid, &status, 0);
	get_excode(sh, status);
}

void piping(t_sh *sh, int nb_pipe)
{
	int pipes[nb_pipe * 2];
	int cpids[nb_pipe + 1];

	int i = 0;
	char **arg;
	while (i < nb_pipe + 1)
	{
		if (ft_strequ(sh->cmd[sh->idx], "|"))
			sh->idx++;
		create_pipe(pipes, nb_pipe, i);
		arg = make_arg(sh);
		cpids[i] = fork();
		if (cpids[i] < 0)
			exit(errmsg(FATAL, NULL));
		if (cpids[i] == 0)
		{
			if (i < nb_pipe)
			{
				if (dup2(pipes[i * 2 + 1], 1) < 0)
					exit(errmsg(FATAL, NULL));
			}
			if (i > 0)
			{
				if (dup2(pipes[(i - 1) * 2], 0) < 0)
					exit(errmsg(FATAL, NULL));
			}
			close_pipe(pipes, nb_pipe, i);
			if (execve(arg[0], arg, sh->env) < 0)
				exit(errmsg(EXE, arg[0]));
		}
		free(arg);
		close_pipe(pipes, nb_pipe, i);
		wait_and_get_excode(sh, cpids[i]);
		i++;
	}
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
		if (execve(arg[0], arg, sh->env) < 0)
			exit(errmsg(EXE, arg[0]));
	}
	else
	{
		free(arg);
		waitpid(cpid, &status, 0);
		get_excode(sh, status);
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