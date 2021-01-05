#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

void usage(const char* proc)
{
	printf("Usage:%s [local_ip] [local_port]\n");
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage(argv[0]); return -1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("error:socket\n"); return -1;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(atoi(argv[2]));
	local.sin_addr.s_addr = inet_addr(argv[1]);

	if (connect(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
	{
		printf("error:connect\n"); return -1;
	}

	printf("client connect success\n");

	char buf[1024];
	while (1)
	{
		printf("client # ");
		fflush(stdout);
		ssize_t s = read(0, buf, sizeof(buf)-1); // 读取0, 输入
		if (s <= 0)
		{
			printf("error:read\n"); return -1;
		}

		buf[s - 1] = 0;    // 回车符删除?
		int fd = dup(1);   // 把文件描述符1复制一份
		dup2(sock, 1);     // 将sock复制一份给描述符1
		printf("%s", buf); // 通过描述符1打印输出
		fflush(stdout);    // 冲洗流
		dup2(fd, 1);       // 将之前的描述符复制回来到1

		s = read(sock, buf, sizeof(buf)-1);
		if (s == 0)
		{
			printf("server quit\n"); break;
		}
		else if (s < 0)
		{
			printf("error:read\n"); return -1;
		}
		else
		{
			buf[s] = 0;
			printf("server # %s\n", buf);
		}
	}

	close(sock);

	return 0;
}
