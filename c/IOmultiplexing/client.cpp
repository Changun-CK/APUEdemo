#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("\nusage:%s ip port\n\n", argv[0]);
		return -1;
	}

	int sockfd;
	char buf[1024];
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(0 != connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))
	{
		printf("connect(%s:%s) failed\n", argv[1], argv[2]);
		return -1;
	}

	printf("connect ok.\n");

	for(int i = 0; i < 10000; ++i)
	{
		// 从命令行输入内容
		memset(buf, 0, sizeof(buf));
		printf("please input:");
		scanf("%s", buf);

		if (write(sockfd, buf, strlen(buf)) <= 0)
		{
			printf("write() failed\n");
			close(sockfd);
			return -1;
		}

		memset(buf, 0, sizeof(buf));
		if (read(sockfd, buf, sizeof(buf)) <= 0)
		{
			printf("read() failed\n");
			close(sockfd);
			return -1;
		}

		printf("recv:%s\n", buf);
	}

	return 0;
}
