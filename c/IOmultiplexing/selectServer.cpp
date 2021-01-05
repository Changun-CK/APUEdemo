#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

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

	fd_set readfdset; // 读事件的集合, 包括监听socket和客户端连接上来的socket
	int maxfd; // readfdset中socket的最大值

	FD_ZERO(&readfdset); // 初始化结构体
	FD_SET(listensock, &readfdset); // 将listensock添加到集合中
	maxfd = listensock;

	while (1)
	{
		// 调用select函数时, 会改变socket集合中内容, 所以要把socket集合保存下来, 传到一个临时的select
		fd_set tmpfdset = readfdset;

		int infds = select(maxfd + 1, &tmpfdset, NULL, NULL, NULL);
		if (infds < 0)
		{
			printf("select() failed\n");
			perror("select()");
			break;
		}

		// 在本程序中, select函数最后一个参数为空, 不存在超时的情况
		if (infds == 0)
		{
			printf("select() timeout\n");
			continue;
		}

		// 检查有事件发生的socket, 包括监听和客户端连接的socket
		// 这里是客户端的socket事件, 每次都要遍历集合, 因为可能有多个socket有事件
		for (int eventfd = 0; eventfd <= maxfd; eventfd++)
		{
			if (FD_ISSET(eventfd, &tmpfdset) <= 0)
			{
				continue;
			}
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

				// 把新客户端socket加入集合
				FD_SET(clientsock, &readfdset);

				if (maxfd < clientsock)
				{
					maxfd = clientsock;
				}
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
					FD_CLR(eventfd, &readfdset); // 集合中移走客户端socket

					// 重新计算maxfd, 只有当eventfd == maxfd时才需要计算
					if (eventfd == maxfd)
					{
						for (int i = maxfd; i > 0; i--)
						{
							if (FD_ISSET(i, &readfdset))
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
