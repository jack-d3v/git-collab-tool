#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define BACKLOG 5

/* Well, yeah
 * Global variables, I know that they are not recommended but in this case I only need it for the signal handler, since it can't take arguments to do something like passing by value or refrence.*/
int server_fd = 0;
int client_sockets[MAX_CLIENTS] = {0};

void handle_sigint(int sig)
{
	(void)sig; // just to suppress the warning about the unused variable

	printf("\nReceived SIGINT signal\n");

	char *message = "The server is shutting down...\n";
	size_t message_len = strlen(message);

	printf("%s", message);
	fflush(stdout);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		int sd = client_sockets[i];

		if (sd > 0)
		{
			send(sd, message, message_len, 0);
			close(sd);
		}
	}

	if (server_fd > 0)
	{
		close(server_fd);
	}

	exit(0);
}

int main(void)
{
	signal(SIGINT, handle_sigint);

	int max_fd;
	int activity;
	int new_socket;
	int valread;

	char buffer[BUFFER_SIZE];


	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		return 1;
	}

	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;


	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		close(server_fd);
		return 1;
	}


	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind");
		close(server_fd);
		return 1;
	}


	if (listen(server_fd, BACKLOG) == -1)
	{
		perror("listen");
		close(server_fd);
		return 1;
	}

	printf("Server is listening on port %d\n", PORT);
	fflush(stdout);



	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);


	fd_set read_fds;


	while (true)
	{
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);
		max_fd = server_fd;

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			int sd = client_sockets[i];

			if (sd > 0)
			{
				FD_SET(sd, &read_fds);
			}

			if (sd > max_fd)
			{
				max_fd = sd;
			}
		}

		activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);


		if (activity == -1)
		{
			perror("select");
			return 1;
		}


		if (FD_ISSET(server_fd, &read_fds))
		{
			new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
			if (new_socket == -1)
			{
				perror("accept");
				return 1;
			}

			printf("Client %d connected\n", new_socket);
			fflush(stdout);


			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_sockets[i] == 0)
				{
					client_sockets[i] = new_socket;
					break;
				}
			}

		}


		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			int sd = client_sockets[i];

			if (FD_ISSET(sd, &read_fds))
			{
				valread = read(sd, buffer, BUFFER_SIZE);

				if (valread == 0)
				{
					printf("Client %d disconnected\n", sd);
					client_sockets[i] = 0;
					fflush(stdout);
					close(sd);
				}

				else
				{
					buffer[valread] = '\0';
					printf("Client %d: %s", sd, buffer);
					fflush(stdout);



					for (int j = 0; j < MAX_CLIENTS; j++)
					{
						int other_sd = client_sockets[j];

						if (other_sd > 0 && other_sd != sd)
						{
							send(other_sd, buffer, valread, 0);
						}
					}
				}


			}
		}



	}

	return 0;
}
