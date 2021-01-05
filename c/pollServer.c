#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define IPADDRESS "127.0.0.1"
#define PORT      5050
#define MAXLINE   1024
#define LISTENQ   5
#define OPEN_MAX  10240 
#define INFTIM    5000

static int socket_bind(const char* ip, int port);
static void do_poll(int listenfd);
static void handle_connection(struct pollfd* connfds, int num);

int main(int argc, char *argv[])
{
	int listenfd = socket_bind(IPADDRESS, PORT);

	listen(listenfd, LISTENQ);

	do_poll(listenfd);

	return 0;
}

static int socket_bind(const char* ip, int port)
{
	int listenfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd == -1)
	{
		perror("socket error"); exit(1);
	}

	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) == -1)
	{
		perror("bind error"); exit(1);
	}

	return listenfd;
}

static void do_poll(int listenfd)
{
	int connfd, sockfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	struct pollfd st_clientfds[OPEN_MAX]; // pollfd结构数组, OPEN_MAX限制了事件的上限
	int maxi = 0;
	int i = 0;
	int nready = 0;

	// 添加监听描述符
	st_clientfds[0].fd = listenfd;
	st_clientfds[0].events = POLLIN;

	// 初始化客户端连接描述符为-1
	for (i = 1; i < OPEN_MAX; ++i)
	{
		st_clientfds[i].fd = -1;
	}

	while (1)
	{
		nready = poll(st_clientfds, maxi+1, INFTIM); // 第二个参数是客户端的连接数
		if (nready == -1)
		{
			perror("poll error"); exit(1);
		}
		else if (nready == 0)
		{
			printf("time out...\n"); continue;
		}

		// 测试监听描述符是否准备好
		if (st_clientfds[0].revents & POLLIN)
		{
			cliaddrlen = sizeof(cliaddr);
			// 接收新的连接
			if ((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen)) == -1)
			{
				if (errno == EINTR) // 被中断的系统调用要重启
					continue;
				else
				{
					perror("accept error"); exit(1);
				}
			}
			fprintf(stdout, "accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
			// 将新的连接描述符添加到数组中
			for (i = 1; i < OPEN_MAX; ++i)
			{
				if (st_clientfds[i].fd < 0)
				{
					st_clientfds[i].fd = connfd;
					break;
				}
			}
			if (i == OPEN_MAX) // 表示坑位不够
			{
				fprintf(stderr, "too many clients.\n");
				exit(1);
			}
			// 将新的描述符添加到数组中
			st_clientfds[i].events = POLLIN;
			// 记录客户连接个数
			maxi = (i > maxi ? i : maxi); // i从小到大排序的
			// 此时为0则超时, 为 1 则只有一个描述符, 即监听描述符
			if (--nready <= 0)
				continue;
		}
		// 处理客户连接
		handle_connection(st_clientfds, maxi);
	}
}

static void handle_connection(struct pollfd* connfds, int num)
{
	int i, n;
	char buf[MAXLINE];
	memset(buf, 0, MAXLINE);
	for (i = 1; i <= num; ++i)
	{
		if (connfds[i].fd < 0)
			continue;
		// 测试客户描述符是否准备好
		if (connfds[i].revents & POLLIN)
		{
			// 接收客户端发送的消息
			n = read(connfds[i].fd, buf, MAXLINE);
			if (n == 0)
			{
				close(connfds[i].fd);
				connfds[i].fd = -1;
				continue;
			}
			//printf("read msg is:");
			write(STDOUT_FILENO, buf, n);
			// 向客户端发送buf
			write(connfds[i].fd, buf, n);
		}
	}
}
