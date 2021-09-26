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
#define POLL_SIZE		1024
#define EPOLL_SIZE		1024

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

void *client_route(void *arg)
{
	int clientfd = (long)arg;

	char buffer[BUFFER_LENGTH + 1] = {0};
	int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
	if (ret < 0)
    {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
			printf("read all data\n");
		}
	}
    else if (ret == 0)
    {
		printf("disconnect \n");
	}
    else
    {
		printf("Recv:%s, %d Bytes\n", buffer, ret);
	}

	close(clientfd);

	pthread_exit(0);
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
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, len); // 重用端口地址
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len); // 保持连接

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
    {
		perror("bind");
		return -1;
	}

	if (listen(sockfd, 5) < 0)
    {
		perror("listen");
		return -1;
	}

#if 0
	// 一连接一线程
	while (1)
    {

		struct sockaddr_in client_addr;
		memset(&client_addr, 0, sizeof(struct sockaddr_in));
		socklen_t client_len = sizeof(client_addr);
		
		int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
		if (clientfd <= 0)
        {
            continue;
        }

		pthread_t thread_id;
		int ret = pthread_create(&thread_id, NULL, client_route, (void*)(long)clientfd);
		pthread_detach(thread_id);
		if (ret < 0)
        {
			perror("pthread_create");
			exit(1);
		}

	}

#elif 0
	// select方法
	fd_set rfds, rset;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	int max_fd = sockfd;
	int i = 0;

	while (1)
    {
		rset = rfds;

		int nready = select(max_fd+1, &rset, NULL, NULL, NULL);
		if (nready < 0)
        {
			printf("select error : %d\n", errno);
			continue;
		}
		if (nready = 0)
		{
			printf("time out...\n");
			continue;
		}

		if (FD_ISSET(sockfd, &rset))
        { //accept
			struct sockaddr_in client_addr;
			memset(&client_addr, 0, sizeof(struct sockaddr_in));
			socklen_t client_len = sizeof(client_addr);

			int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
			if (clientfd <= 0)
            {
                continue;
            }

			char str[INET_ADDRSTRLEN] = {0};
			printf("recvived from %s at port %d, sockfd:%d, clientfd:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
				ntohs(client_addr.sin_port), sockfd, clientfd);

			if (max_fd == FD_SETSIZE)
            {
				printf("clientfd --> out range\n");
				break;
			}
			FD_SET(clientfd, &rfds);

			if (clientfd > max_fd)
            {
                max_fd = clientfd;
            }

			printf("sockfd:%d, max_fd:%d, clientfd:%d\n", sockfd, max_fd, clientfd);

			if (--nready == 0)
            {
                continue;
            }
		}

		for (i = sockfd + 1; i <= max_fd; i++)
        {
			if (FD_ISSET(i, &rset))
            {
				char buffer[BUFFER_LENGTH + 1] = {0};
				int ret = recv(i, buffer, BUFFER_LENGTH, 0);
				if (ret < 0)
                {
					if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
						printf("read all data");
					}
					FD_CLR(i, &rfds);
					close(i);
				}
                else if (ret == 0)
                {
					printf(" disconnect %d\n", i);
					FD_CLR(i, &rfds);
					close(i);
					break;
				}
                else
                {
					printf("Recv: %s, %d Bytes\n", buffer, ret);
				}
				if (--nready == 0)
                {
                    break;
                }
			}
		}
	}

#elif 0
	// poll方法
	struct pollfd fds[POLL_SIZE] = {0};
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	int max_fd = 0, i = 0;
	for (i = 1; i < POLL_SIZE; i++)
    {
		fds[i].fd = -1;
	}

	while (1)
    {
		int nready = poll(fds, max_fd+1, 5);
		if (nready <= 0)
        {
            continue;
        }

		if ((fds[0].revents & POLLIN) == POLLIN)
        {
			struct sockaddr_in client_addr;
			memset(&client_addr, 0, sizeof(struct sockaddr_in));
			socklen_t client_len = sizeof(client_addr);
		
			int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
			if (clientfd <= 0)
            {
                continue;
            }

			char str[INET_ADDRSTRLEN] = {0};
			printf("recvived from %s at port %d, sockfd:%d, clientfd:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
				ntohs(client_addr.sin_port), sockfd, clientfd);

			fds[clientfd].fd = clientfd;
			fds[clientfd].events = POLLIN;

			if (clientfd > max_fd)
            {
                max_fd = clientfd;
            }

			if (--nready == 0)
            {
                continue;
            }
		}

		for (i = sockfd + 1; i <= max_fd; i++)
        {
			if (fds[i].revents & (POLLIN|POLLERR))
            {
				char buffer[BUFFER_LENGTH + 1] = {0};
				int ret = recv(i, buffer, BUFFER_LENGTH, 0);
				if (ret < 0)
                {
					if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
						printf("read all data");
					}
					
					close(i);
					fds[i].fd = -1;
				}
                else if (ret == 0)
                {
					printf(" disconnect %d\n", i);
					
					close(i);
					fds[i].fd = -1;
					break;
				}
                else
                {
					printf("Recv: %s, %d Bytes\n", buffer, ret);
				}
				if (--nready == 0)
                {
                    break;
                }
			}
		}
	}

#else
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
		for (i = 0; i < nready; i++)
        {
			int iSockfd = events[i].data.fd;
			if (iSockfd == sockfd)
            {
				struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(struct sockaddr_in));
				socklen_t client_len = sizeof(client_addr);
			
				int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
				if (clientfd <= 0)
                {
                    continue;
                }
	
				char str[INET_ADDRSTRLEN] = {0};
				printf("recvived from %s at port %d, sockfd:%d, clientfd:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
					ntohs(client_addr.sin_port), sockfd, clientfd);

				// 对每个非监听文件描述符都要注册EPOLLONESHOT事件, 这是为了线程安全, 虽然此程序没有采用多线程处理每个连接
				epollAddFd(epoll_fd, clientfd, 1);

				//ev.events = EPOLLIN | EPOLLET; // 对客户端socket用边缘触发
				//ev.data.fd = clientfd;
				//setNonBlocking(clientfd);      // 新客户端注册时设置为非阻塞IO
				//epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientfd, &ev);
			}
            else if (events[i].events & EPOLLIN)
            {
				char buffer[BUFFER_LENGTH + 1];

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
					}
				}
			}
			else
            {
				printf("something else happend!\n");
			}
		}
	}

	close(epoll_fd);
#endif

	close(sockfd);

	return 0;
}
