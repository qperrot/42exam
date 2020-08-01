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

enum e_err
{
	ARG,
	FATAL,
};

typedef struct s_server
{
	int socket;
	int assign_id;
	struct sockaddr_in addr;
} t_server;

typedef struct s_client
{
	int id;
	int socket;
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

uint16_t port_to_big_endian(uint16_t port)
{
	// ex) 80 port little endian : 01010000  / 00000000
	return (port >> 8 | port << 8);
}

uint32_t localhost_to_big_endian()
{
	// ex) 127.0.0.1 little endian	: 00000001 00000000 00000000 01111111  = 2130706433
	// after change to big endian	: 01111111 00000000 00000000 00000001
	unsigned int ip = 2130706433;
	return (ip >> 24 | ip << 24);
}

void lst_add_client(t_client *new_cli)
{
	t_client *cli = get_clients();

	while (cli->nxt)
		cli = cli->nxt;
	cli->nxt = new_cli;
	new_cli->id = get_server()->assign_id++;
}

void lst_remove_client(int socket)
{
	t_client *bef = get_clients();
	t_client *cli = bef->nxt;

	while (cli)
	{
		if (cli->socket == socket) {
			bef->nxt = cli->nxt;
			free(cli);
			return;
		}
		bef = cli;
		cli = cli->nxt;
	}
}

int get_fdmax()
{
	int max = get_server()->socket;
	t_client *cli = get_clients();
	while (cli)
	{
		if (max < cli->socket)
			max = cli->socket;
		cli = cli->nxt;
	}
	return max;
}

void manage_server(fd_set *read_set, fd_set *write_set, fd_set *init_set)
{
	t_server *srv = get_server();
	t_client *cli_begin = get_clients();

	if (FD_ISSET(srv->socket, read_set))
	{
		t_client *new_cli = calloc(1, sizeof(t_client));
		new_cli->socket = accept(srv->socket, (struct sockaddr *)&new_cli->addr, sizeof(new_cli->addr));
		if (new_cli->socket < 0) {
			free(new_cli);
			return;
		}
		if (fcntl(new_cli->socket, F_SETFL, O_NONBLOCK) < 0) {
			free(new_cli);
			return;
		}
		lst_add_client(new_cli);
		FD_SET(new_cli->socket, init_set);
	}
}

void manage_clients(fd_set *read_set, fd_set *write_set, fd_set *init_set)
{
	t_client *cli = get_clients()->nxt;

	while (cli)
	{
		if (FD_ISSET(cli->socket, read_set))
		{
			ssize_t nb_read = recv(cli->socket, )
		}
	}
}

int main(int ac, char **av)
{
	if (ac == 1)
		eprint(ARG);

	t_server *server = get_server();

	// create server socket
	if (server->socket = socket(AF_INET, SOCK_STREAM, 0) < 0)
		eprint(FATAL);

	// bind IP/port to socket
	// Network byte order : big endian
	// Ours byte order : little endian
	server->addr.sin_family = AF_INET;
	server->addr.sin_addr.s_addr = localhost_to_big_endian(); // uint32_t
	server->addr.sin_port = port_to_big_endian(atoi(av[1]));  // uint16_t
	if (bind(server->socket, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0)
		eprint(FATAL);

	// listen
	if (listen(server->socket, 100) < 0)
		eprint(FATAL);

	// prepare fdset
	fd_set read_set, write_set, init_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&init_set);
	FD_SET(server->socket, &init_set);

	struct timeval timeout;

	// start to run server
	while (1)
	{
		read_set = init_set;
		write_set = init_set;

		// select
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		if (select(get_fdmax() + 1, &read_set, &write_set, NULL, &timeout) < 0)
			eprint(FATAL);

		// manage client and server
		manage_clients(&read_set, &write_set, &init_set); // read/write
		manage_server(&read_set, &write_set, &init_set);  // accept
	}
	return (0);
}