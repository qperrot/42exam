#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>

enum e_def
{
	ARG,
	FATAL,
	LOGIN,
	LOGOUT,
	CHAT,
	MAX_BUFF = 4096
};

typedef struct s_server
{
	int socket;
	int assign_id;
	struct sockaddr_in addr;
} t_server;

typedef struct s_client
{
	int socket;
	int id;
	struct sockaddr_in addr;
	struct s_client *nxt;
	int status;
	char *read;
} t_client;

t_server srv;
fd_set init_set, read_set, write_set;

t_client *get_client()
{
	static t_client cli;
	return &cli;
}

void eprint(int err)
{
	char *s;
	if (err == ARG)
		s = "Wrong number of arguments\n";
	if (err == FATAL)
		s = "Fatal error\n";
	write(2, s, strlen(s));
	exit(1);
}

int get_fdmax()
{
	int max = srv.socket;
	t_client *cli = get_client()->nxt;

	while (cli)
	{
		if (cli->socket > max)
			max = cli->socket;
		cli = cli->nxt;
	}
	return max;
}

void lst_add_client(t_client *new)
{
	t_client *cli = get_client();
	new->id = srv.assign_id++;
	new->read = calloc(1, 1);
	if (!new->read)
		eprint(FATAL);
	while (cli->nxt)
		cli = cli->nxt;
	cli->nxt = new;
}

void lst_remove_client(t_client *rm)
{
	t_client *bef = get_client();
	t_client *cli = bef->nxt;

	while (cli)
	{
		if (cli->id == rm->id)
		{
			bef->nxt = rm->nxt;
			close(rm->socket);
			free(rm->read);
			free(rm);
			return;
		}
		bef = cli;
		cli = cli->nxt;
	}
}

void manage_server()
{
	if (FD_ISSET(srv.socket, &read_set))
	{
		t_client *new = calloc(1, sizeof(t_client));
		if (!new)
			eprint(FATAL);
		socklen_t len = sizeof(new->addr);
		new->socket = accept(srv.socket, (struct sockaddr *)&new->addr, &len);
		if (new->socket < 0)
			eprint(FATAL);
		lst_add_client(new);
		FD_SET(new->socket, &init_set);
		new->status = LOGIN;
	}
}

char *extract(char *source, int len)
{
	char *res = malloc(len + 1);
	if (!res)
		eprint(FATAL);
	int i = 0;
	while (i < len)
	{
		res[i] = source[i];
		i++;
	}
	res[i] = 0;
	return res;
}

void pop(char **source, int len)
{
	char *s = *source;
	strcpy(s, s + len);
	s = realloc(s, strlen(s) + 1);
	if (!s)
		eprint(FATAL);
	*source = s;
}

void append(char **dest, char *s)
{
	char *d = *dest;
	d = realloc(d, strlen(d) + strlen(s) + 1);
	if (!d)
		eprint(FATAL);
	strcat(d, s);
	*dest = d;
}

void broadcast(t_client *og, char *buff)
{
	t_client *cli = get_client()->nxt;

	while (cli)
	{
		if (cli->id != og->id && cli->status != LOGIN && cli->status != LOGOUT && FD_ISSET(cli->socket, &write_set))
		{
			int nb_send = send(cli->socket, buff, strlen(buff), MSG_DONTWAIT);
			// need to check ... ?
		}
		cli = cli->nxt;
	}
}

void manage_irc()
{
	t_client *cli = get_client()->nxt;
	char buff[MAX_BUFF + 50];
	char *p;

	while (cli)
	{
		if (cli->status == LOGIN)
		{
			sprintf(buff, "server: client %d just arrived\n", cli->id);
			broadcast(cli, buff);
		}
		else if (cli->status == LOGOUT)
		{
			sprintf(buff, "server: client %d just left\n", cli->id);
			broadcast(cli, buff);
			t_client *rm = cli;
			cli = cli->nxt;
			FD_CLR(rm->socket, &init_set);
			lst_remove_client(rm);
			continue;
		}
		else if (cli->status == CHAT && (p = strstr(cli->read, "\n")))
		{
			int len = p - cli->read + 1;
			char *s = extract(cli->read, len);
			pop(&cli->read, len);
			sprintf(buff, "client %d: %s", cli->id, s);
			free(s);
			broadcast(cli, buff);
		}
		cli->status = 0;
		cli = cli->nxt;
	}
}

void manage_client()
{
	t_client *cli = get_client()->nxt;
	char buff[MAX_BUFF + 1];

	while (cli)
	{
		if (FD_ISSET(cli->socket, &read_set))
		{
			int nb_read = recv(cli->socket, buff, MAX_BUFF, MSG_DONTWAIT);
			if (nb_read == 0)
				cli->status = LOGOUT;
			else if (nb_read > 0)
			{
				buff[nb_read] = 0;
				append(&cli->read, buff);
				cli->status = CHAT;
			}
		}
		cli = cli->nxt;
	}
}

int main(int ac, char **av)
{
	if (ac == 1)
		eprint(ARG);
	if ((srv.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		eprint(FATAL);
	uint32_t localhost = 2130706433;
	uint16_t port = atoi(av[1]);
	srv.addr.sin_family = AF_INET;
	srv.addr.sin_addr.s_addr = localhost >> 24 | localhost << 24;
	srv.addr.sin_port = port >> 8 | port << 8;
	if (bind(srv.socket, (const struct sockaddr *)&srv.addr, sizeof(srv.addr)) < 0)
		eprint(FATAL);
	if (listen(srv.socket, 10) < 0)
		eprint(FATAL);
	FD_SET(srv.socket, &init_set);
	while (1)
	{
		read_set = init_set;
		write_set = init_set;
		if (select(get_fdmax() + 1, &read_set, &write_set, NULL, NULL) < 0)
			eprint(FATAL);
		manage_server();
		manage_client();
		manage_irc();
	}
	return 0;
}
