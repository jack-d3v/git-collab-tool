#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define MESSAGE "Hello Client!"

int main(void)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		return 1;
	}

	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind");
		close(sockfd);
		return 1;
	}


	if (listen(sockfd, 1) == -1)
	{
		perror("listen");
		close(sockfd);
		return 1;
	}

	printf("Server is listening on port %d\n", PORT);
	fflush(stdout);


	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
	if (clientfd == -1)
	{
		perror("accept");
		close(sockfd);
		return 1;
	}

	send(clientfd, MESSAGE, strlen(MESSAGE) - 1, 0);

	close(clientfd);
	close(sockfd);
	

	return 0;
}
