#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define SIZE sizeof(fd_set)*8
int readfds[SIZE];

void usage(const char* proc);
int startup(const char* ip, int port);

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage(argv[0]); return -1;
	}
	printf("size:%d\n", SIZE);

	int sock = startup(argv[1], atoi(argv[2]));

	unsigned long i = 0;
	for (; i < SIZE; ++i)
	{
		readfds[i] = -1; // 全部初始化为-1
	}

	readfds[0] = sock; // 监听用的socket设置为第一位

	int max_fd = readfds[0]; // 保存最大的文件描述符用作select函数参数

	while (1)
	{
		fd_set rfds, wfds; // 创建读集和写集
		char buf[1024];
		FD_ZERO(&rfds); // 将描述符集指针指向的文件描述符清零
		unsigned long j = 0;
		for (; j < SIZE; ++j)
		{
			if (readfds[j] != -1)
			{
				// 将文件描述符设置进文件描述符集指针指向的文件描述符集中
				FD_SET(readfds[j], &rfds);
			}
			if (max_fd < readfds[j])
			{
				max_fd = readfds[j];
			}
		}

		struct timeval timeout = {5, 0}; // 初始化时间结构体, 5s超时
		switch(select(max_fd+1, &rfds, &wfds, NULL, &timeout))
		{
			case -1:
			{
				printf("error:select\n"); break;
			}
			case 0:
			{
				printf("timeout...\n"); break;
			}
			default:
			{
				unsigned long k = 0;
				for (; k < SIZE; ++k)
				{
					// FD_ISSET用来判断文件描述符是否在文件描述符集中, 1-是, 0-否
					// 这里首先是监听的socket, 来监听新的socket
					if (readfds[k] == sock && FD_ISSET(readfds[k], &rfds))
					{
						struct sockaddr_in peer;
						socklen_t len = sizeof(struct sockaddr);
						int newsock = accept(sock, (struct sockaddr*)&peer, &len);
						if (newsock < 0)
						{
							printf("error:accept\n"); continue;
						}

						unsigned long l = 0;
						for (; l < SIZE; ++l) // 找最近的空位加入新的socket
						{
							if (readfds[l] == -1)
							{
								readfds[l] = newsock; break;
							}
						}
						if (l == SIZE) // 没有空位容纳新socket
						{
							printf("error:readfds is full\n");
							return -1;
						}
					}
					// 读文件描述符的内容
					else if (readfds[k] > 0 && FD_ISSET(readfds[k], &rfds))
					{
						ssize_t s = read(readfds[k], buf, sizeof(buf) - 1);
						if (s < 0)
						{
							printf("error:read\n"); return -1;
						}
						else if (s == 0)
						{
							printf("client quit\n");
							readfds[k] = -1;
							close(readfds[k]);
							continue;
						}
						else
						{
							buf[s] = 0;
							printf("client # %s\n", buf);
							fflush(stdout);
							write(readfds[k], buf, strlen(buf));
						}
					}
				}
			}
			break;
		}
	}

	close(sock);

	return 0;
}

void usage(const char* proc)
{
	printf("Usage:%s [local_ip] [local_port]\n");
}

int startup(const char* ip, int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("error:socket\n"); return 2;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

	if (bind(sock, (struct sockaddr*)&local, sizeof(struct sockaddr_in)) < 0)
	{
		printf("error:bind\n");
		return 3;
	}

	if (listen(sock, 5) < 0)
	{
		printf("error:listen\n");
		return 4;
	}

	return sock;
}
