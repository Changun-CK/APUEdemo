#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

#define MAXNFDS 1024

int initServer(int port);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("\nusage:%s port\n\n", argv[0]);
		return -1;
	}

	// 初始化服务端监听的socket
	int listensock = initServer(atoi(argv[1]));
	printf("listensock = %d\n", listensock);
	if (listensock < 0)
	{
		printf("initserver() failed.\n");
		return -1;
	}

	struct pollfd fds[MAXNFDS]; // fds中存放需要监听的socket
	int maxfd; // fds数组中需要监视的socket的大小

	for (int i = 0; i < MAXNFDS; ++i)
	{
		fds[i].fd = -1; // 初始化结构体数组, 全部fd为-1
	}

	// 把listensock添加到数组中
	fds[listensock].fd = listensock;
	fds[listensock].events = POLLIN;
	maxfd = listensock;

	while (1)
	{
		int infds = poll(fds, maxfd + 1, -1);
		if (infds < 0)
		{
			printf("poll() failed\n");
			perror("poll():");
			break;
		}

		// 超时
		if (infds == 0)
		{
			printf("poll() timeout=\n");
			continue;
		}

		// 检查有事件发生的socket, 包括监听和客户端连接的socket
		// 这里是客户端的socket事件, 每次都要遍历集合, 因为可能有多个socket有事件
		for (int eventfd = 0; eventfd <= maxfd; eventfd++)
		{
			if (fds[eventfd].fd < 0)
			{
				continue;
			}
			if ((fds[eventfd].revents & POLLIN) == 0)
			{
				continue;
			}

			fds[eventfd].revents = 0; // 先将revents清空

			if (eventfd == listensock)
			{
				// 如果是listensock, 表示有新的客户端连接上来
				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				int clientsock = accept(listensock, (struct sockaddr *)&client, &len);
				if (clientsock < 0)
				{
					printf("accept() failed.\n");
					continue;
				}

				printf("client(socket=%d) connect ok.\n", clientsock);

				if (MAXNFDS < clientsock)
				{
					printf("clientsock(%d) > MAXNFDS(%d)\n", clientsock, MAXNFDS);
					continue;
				}

				fds[clientsock].fd = clientsock;
				fds[clientsock].events = POLLIN;
				fds[clientsock].revents = 0;

				if (maxfd < clientsock)
				{
					maxfd = clientsock;
				}
				printf("maxfd = %d\n", maxfd);
				continue;
			}
			else
			{
				// 客户端有数据过来或客户端的socket连接被断开
				char buffer[1024];
				memset(buffer, 0, sizeof(buffer));
				// 读取客户端数据
				ssize_t isize = read(eventfd, buffer, sizeof(buffer));

				// 发生了错误或socket被对方关闭
				if (isize <= 0)
				{
					printf("client(=%d) disconnected.\n", eventfd);
					close(eventfd);
					fds[eventfd].fd = -1;

					// 重新计算maxfd, 只有当eventfd == maxfd时才需要计算
					if (eventfd == maxfd)
					{
						for (int i = maxfd; i > 0; i--)
						{
							if (fds[i].fd != -1)
							{
								maxfd = i;
								break;
							}
						}
						printf("maxfd == %d\n", maxfd);
					}
					continue;
				}
				printf("recv(eventfd = %d, size = %d):%s\n", eventfd, isize, buffer);
				// 把收到的报文发回给客户端
				write(eventfd, buffer, strlen(buffer));
			}
		}
	}

	return 0;
}

int initServer(int port)
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed\n");
		return -1;
	}

	int opt = 1;
	unsigned int len = sizeof(opt);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len);

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("bind() failed\n");
		return -1;
	}

	if (0 != listen(sockfd, 5))
	{
		printf("listen() failed.\n");
		close(sockfd);
		return -1;
	}

	return sockfd;
}
