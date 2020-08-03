// write, close, select, socket, accept, listen, send, recv, 
// bind, strstr, malloc, realloc, free, calloc, bzero, atoi, 
// sprintf, strlen, exit, strcpy, strcat, memset

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h> // IPPROTO_TCP
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

enum e_define
{
	ARG,
	FATAL,
	LOGIN,
	LOGOUT,
	CHAT
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
	char *data;
	struct sockaddr_in addr;
	struct s_client *nxt;
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
	exit(EXIT_FAILURE);
}

void lst_add_client(t_client *new_cli)
{
	t_client *cli = get_clients();

	new_cli->id = get_server()->assign_id++;
	new_cli->data = calloc(1, 1);
	if (!new_cli->data)
		eprint(FATAL);

	while (cli->nxt)
		cli = cli->nxt;
	cli->nxt = new_cli;
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
			free(rm->data);
			free(rm);
			return;
		}
		bef = cli;
		cli = cli->nxt;
	}
}

int get_fdmax()
{
	int max = get_server()->socket;
	t_client *cli = get_clients()->nxt;
	while (cli)
	{
		if (max < cli->socket)
			max = cli->socket;
		cli = cli->nxt;
	}
	return max;
}

void broadcast(int type, t_client *sender, char *s)
{
	t_client *cli = get_clients()->nxt;
	char buff[4500];
	char *new_data;

	if (type == LOGIN)
		sprintf(buff, "server: client %d just arrived\n", sender->id);
	else if (type == LOGOUT)
		sprintf(buff, "server: client %d just left\n", sender->id);
	else if (type == CHAT)
		sprintf(buff, "client %d: %s", sender->id, s);

	while (cli)
	{
		if ((type == CHAT || type == LOGOUT) && cli->id == sender->id) {
			cli = cli->nxt;
			continue;
		}
		char *res = malloc(strlen(cli->data) + strlen(buff) + 1);
		if (!res)
			eprint(FATAL);
		res[0] = 0;
		strcat(res, cli->data);
		strcat(res, buff);
		free(cli->data);
		cli->data = res;

		cli = cli->nxt;
	}
}

void manage_server(fd_set *read_set, fd_set *write_set, fd_set *init_set)
{
	t_server *srv = get_server();
	t_client *cli_begin = get_clients();

	if (FD_ISSET(srv->socket, read_set))
	{
		t_client *new_cli = calloc(1, sizeof(t_client));
		int len = sizeof(struct sockaddr_in);
		new_cli->socket = accept(srv->socket, (struct sockaddr *)&new_cli->addr, &len);
		if (new_cli->socket < 0) {
			free(new_cli);
			return;
		}
		if (fcntl(new_cli->socket, F_SETFL, O_NONBLOCK) < 0) {
			free(new_cli);
			return;
		}
		lst_add_client(new_cli);
		broadcast(LOGIN, new_cli, NULL);
		FD_SET(new_cli->socket, init_set);
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
	char buff[4096 + 1];

	while (cli)
	{
		if (FD_ISSET(cli->socket, read_set))
		{
			ssize_t nb_read = recv(cli->socket, buff, 4096, 0);
			// connection closed by client
			if (nb_read == 0) {
				broadcast(LOGOUT, cli, NULL);
				disconnect(cli, init_set);
				return;
			}
			// data from client side
			if (nb_read > 0) {
				buff[nb_read] = 0;
				broadcast(CHAT, cli, buff);
			}
		}
		if (FD_ISSET(cli->socket, write_set) && cli->data)
		{
			int nb_send = send(cli->socket, cli->data, strlen(cli->data), MSG_NOSIGNAL);
			if (nb_send >= 0) {
				char *s = cli->data;
				int i = nb_send;
				int j = 0;
				while (s[i])
					cli->data[j++] = s[i++];
				cli->data[j] = 0;
				cli->data = realloc(cli->data, strlen(cli->data) + 1);
			}
		}
		cli = cli->nxt;
	}
}

int main(int ac, char **av)
{
	if (ac == 1)
		eprint(ARG);

	t_server *server = get_server();

	// create server socket
	if ((server->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		eprint(FATAL);

	// bind IP/port to socket
	uint32_t localhost = 2130706433;
	uint16_t port = atoi(av[1]);
	server->addr.sin_family = AF_INET;
	server->addr.sin_addr.s_addr = (localhost >> 24 | localhost << 24); // uint32_t
	server->addr.sin_port = (port >> 8 | port << 8);  // uint16_t
	if (bind(server->socket, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0)
		eprint(FATAL);

	// listen
	if (listen(server->socket, 100) < 0)
		eprint(FATAL);

	// prepare fdset
	fd_set read_set, write_set, init_set;
	FD_ZERO(&init_set);
	FD_SET(server->socket, &init_set);

	// start to run server
	while (1)
	{
		read_set = init_set;
		write_set = init_set;
		if (select(get_fdmax() + 1, &read_set, &write_set, NULL, NULL) < 0)
			eprint(FATAL);
		manage_clients(&read_set, &write_set, &init_set);
		manage_server(&read_set, &write_set, &init_set);
	}
	return (0);
}