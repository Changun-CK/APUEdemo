#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>


#define BUFFER_LENGTH	5
#define EPOLL_SIZE		1024

struct fds
{
	int epoll_fd;
	int clientfd;
};

/* 设置文件描述符为非阻塞 */
int setNonBlocking(int sockfd)
{
	// 先获取fd的属性再和非阻塞属性做或运算
	if (-1 == fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK))
	{
		return -1;
	}

	return 0;
}

// 我们希望在任何时候sockfd都由一个线程处理, 所以采用EPOLLONESHOT事件, 保证线程安全
// oneshot 指定是否注册fd上的EPOLLONESHOT事件
// 尤其注意监听socketfd不能注册EPOLLONESHOT事件, 否则应用程序只能处理一个客户连接! 因为后续的客户连接请求将不再触发listenfd上的EPOLLIN事件
void epollAddFd(int epollfd, int fd, int oneshot)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	setNonBlocking(fd);
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

// 重置fd上的事件, 这样操作后, 尽管fd上的EPOLLONESHOT事件被注册, 但是操作系统仍然会触发fd上的EPOLLIN事件, 且只触发一次
void reset_oneshot(int epollfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

// 线程函数
void *proc(void *arg)
{
	int iSockfd = ((struct fds *)arg)->clientfd;
	int epoll_fd = ((struct fds *)arg)->epoll_fd;

	struct epoll_event ev;
	char buffer[BUFFER_LENGTH + 1];
	printf("new thread id=%d, receive data on fd=%d\n", pthread_self(), iSockfd);

	while (1) // 这个循环必不可少, 边缘触发确保读完所有数据
	{
		memset(buffer, 0, BUFFER_LENGTH + 1);
		int ret = recv(iSockfd, buffer, BUFFER_LENGTH, 0);
		if (ret < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				printf("loop read all data\n");
				// 重置iSockfd的事件 
				reset_oneshot(epoll_fd, iSockfd);
				break;
			}
			
			close(iSockfd);
			
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = iSockfd;
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, iSockfd, &ev);
			break;
		}
		else if (ret == 0)
		{
			printf(" disconnect %d\n", iSockfd);
			
			close(iSockfd);

			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = iSockfd;
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, iSockfd, &ev);

			break;
		}
		else
		{
			printf("Recv: %s, %d Bytes\n", buffer, ret);
			/* 休眠5s, 模拟数据处理过程, 由于buffer接收长度设置最大为5, 只要超过5字节的数据便可观察现象 */
			sleep(5);
		}
	}
}


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Paramter Error\n");
		return -1;
	}
	int port = atoi(argv[1]);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket");
		return -1;
	}

    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		return 2;
	}

	if (listen(sockfd, 5) < 0)
	{
		perror("listen");
		return 3;
	}

	// epoll方法, 原理如同小区蜂巢, 监听描述符采用水平触发, 客户端采用边缘触发非阻塞IO
	int epoll_fd = epoll_create(EPOLL_SIZE);
	struct epoll_event ev, events[EPOLL_SIZE] = {0};

	// 将监听socket加入ev, ev代表当前处理的socket
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev);

	while (1)
	{
		int nready = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
		if (nready == -1)
		{
			printf("error:epoll_wait\n");
			break;
		}

		int i = 0;
		for (i = 0;i < nready;i ++)
		{

			int iSockfd = events[i].data.fd;
			if (iSockfd == sockfd)
			{
				struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(struct sockaddr_in));
				socklen_t client_len = sizeof(client_addr);
			
				int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
				if (clientfd <= 0) continue;
	
				char str[INET_ADDRSTRLEN] = {0};
				printf("recvived from %s at port %d, sockfd:%d, clientfd:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
					ntohs(client_addr.sin_port), sockfd, clientfd);

				// 对每个非监听文件描述符都要注册EPOLLONESHOT事件, 这是为了线程安全,
				epollAddFd(epoll_fd, clientfd, 1);

			}
			else if (events[i].events & EPOLLIN)
			{
				pthread_t pthread_id;
				struct fds iFds;
				iFds.epoll_fd = epoll_fd;
				iFds.clientfd = iSockfd;
				// 启动一个线程处理数据
				int ret = pthread_create(&pthread_id, NULL, proc, (void *)&iFds);
				if (ret != 0)
				{
					printf("线程创建失败,clientfd=%d\n", iSockfd);
				}
			}
			else
			{
				printf("something else happend!\n");
			}
		}

	}

	close(epoll_fd);
	close(sockfd);

	return 0;
}
