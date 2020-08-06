#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>

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
	char *read;
	char *write;
} t_client;

t_server *get_server()
{
	static t_server srv;
	return &srv;
}

t_client *get_clients()
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
	close(get_server()->socket);
	perror("");
	exit(EXIT_FAILURE);
}

void lst_add_client(t_client *new)
{
	t_client *cli = get_clients();

	new->id = get_server()->assign_id++;
	new->read = calloc(1, 1);
	new->write = calloc(1, 1);
	if (!new->read || !new->write)
		eprint(FATAL);
	while (cli->nxt)
		cli = cli->nxt;
	cli->nxt = new;
}

void lst_remove_client(t_client *rm)
{
	t_client *bef = get_clients();
	t_client *cli = bef->nxt;

	while (cli)
	{
		if (cli->socket == rm->socket)
		{
			bef->nxt = cli->nxt;
			free(rm->read);
			free(rm->write);
			free(rm);
			return;
		}
		bef = cli;
		cli = cli->nxt;
	}
}

int get_fdmax()
{
	t_server *srv = get_server();
	t_client *cli = get_clients();

	while (cli->nxt)
		cli = cli->nxt;
	return (srv->socket > cli->socket ? srv->socket : cli->socket);
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

// 앞에서부터 얼마만큼 추출할 건지
void pop(char **source, int len)
{
	int i = 0;
	char *s = *source;

	while (s[i + len])
	{
		s[i] = s[i + len];
		i++;
	}
	s[i] = 0;
	s = realloc(s, i + 1);
	if (!s)
		eprint(FATAL);
	*source = s;
}

char *extract(char *source, int len)
{
	char *res = malloc(len + 1);
	if (!res)
		eprint(FATAL);
	// res 로 복사
	int i = 0;
	while (i < len)
	{
		res[i] = source[i];
		i++;
	}
	res[i] = 0;
	return res;
}

void broadcast(int type, t_client *org)
{
	t_client *cli = get_clients()->nxt;
	char buff[4500];

	if (type == LOGIN)
		sprintf(buff, "server: client %d just arrived\n", org->id);
	else if (type == LOGOUT)
		sprintf(buff, "server: client %d just left\n", org->id);
	else if (type == CHAT) {
		int len = strstr(org->read, "\n") - org->read + 1;
		char *s = extract(org->read, len);
		pop(&org->read, len);
		sprintf(buff, "client %d: %s", org->id, s);
		free(s);
	}

	while (cli)
	{
		if (cli->id == org->id)
		{
			cli = cli->nxt;
			continue;
		}
		append(&cli->write, buff);
		cli = cli->nxt;
	}
}

void manage_server(fd_set *read_set, fd_set *init_set)
{
	t_server *srv = get_server();

	if (FD_ISSET(srv->socket, read_set))
	{
		t_client *new = calloc(1, sizeof(t_client));
		if (!new)
			eprint(FATAL);
		int len = sizeof(struct sockaddr_in);
		new->socket = accept(srv->socket, (struct sockaddr *)&new->addr, (socklen_t*)&len);
		if (new->socket < 0 || fcntl(new->socket, F_SETFL, O_NONBLOCK) < 0)
		{
			free(new);
			close(new->socket);
			return;
		}
		lst_add_client(new);
		broadcast(LOGIN, new);
		FD_SET(new->socket, init_set);
	}
}

void disconnect(t_client *cli, fd_set *init_set)
{
	close(cli->socket);
	FD_CLR(cli->socket, init_set);
	lst_remove_client(cli);
}

void manage_clients(fd_set *read_set, fd_set *write_set, fd_set *init_set)
{
	t_client *cli = get_clients()->nxt;
	char buff[MAX_BUFF + 1];

	while (cli)
	{
		// broadcasting
		if (strstr(cli->read, "\n"))
			broadcast(CHAT, cli);
		// write
		if (FD_ISSET(cli->socket, write_set) && cli->write[0])
		{
			int nb_send = send(cli->socket, cli->write, strlen(cli->write), MSG_NOSIGNAL);
			if (nb_send >= 0)
				pop(&cli->write, nb_send);
		}
		// read
		if (FD_ISSET(cli->socket, read_set))
		{
			int nb_read = recv(cli->socket, buff, 4096, 0);
			// connection closed by client
			if (nb_read == 0)
			{
				broadcast(LOGOUT, cli);
				disconnect(cli, init_set);
				return;
			}
			// append to read
			if (nb_read > 0)
			{
				buff[nb_read] = 0;
				append(&cli->read, buff);
			}
		}
		cli = cli->nxt;
	}
}

int main(int ac, char **av)
{
	t_server *srv = get_server();

	if (ac == 1)
		eprint(ARG);

	if ((srv->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		eprint(FATAL);

	uint32_t localhost = 2130706433;
	uint16_t port = atoi(av[1]);
	srv->addr.sin_family = AF_INET;
	srv->addr.sin_addr.s_addr = (localhost >> 24 | localhost << 24); // uint32_t
	srv->addr.sin_port = (port >> 8 | port << 8);  // uint16_t

	if (bind(srv->socket, (struct sockaddr *)&srv->addr, sizeof(srv->addr)) < 0)
		eprint(FATAL);

	if (listen(srv->socket, 100) < 0)
		eprint(FATAL);

	fd_set read_set, write_set, init_set;
	FD_ZERO(&init_set);
	FD_SET(srv->socket, &init_set);

	while (1)
	{
		read_set = init_set;
		write_set = init_set;
		if (select(get_fdmax() + 1, &read_set, &write_set, NULL, NULL) < 0)
			eprint(FATAL);
		manage_clients(&read_set, &write_set, &init_set);
		manage_server(&read_set, &init_set);
	}
	return (0);
}