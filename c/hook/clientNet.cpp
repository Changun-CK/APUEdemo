#include <stdlib.h>

#include "filterNet.h"

#define STRLEN 1024

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("\nplease input 3 parameter\n");
		printf("example:./a.out [ip] [port]\n\n");
		return -1;
	}
	char buffer[STRLEN];
	char ip[32];
	strcpy(ip, argv[1]);
	int port = atoi(argv[2]);

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port); 
	inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);

    // 将hook打开, 禁止连接
    setFilterIp(argv[1]);
    filterNet::set_hook_enable(true);
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
        printf("第一次连接失败\n");
        // 注意, 为了测试, 第一次连接失败, 说明hook成功, 不要关闭socket
		//close(sockfd);
        //return -1;
	}

    // 将hook关闭, 放行
    filterNet::set_hook_enable(false);
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
        // 第二次不会失败, 说明我们放行成功
        printf("第二次连接失败\n");
		close(sockfd);
        return -1;
	}

	while (1)
	{
		printf("input:");
		memset(buffer,0,sizeof(buffer));
		scanf("%s", buffer);
		if (send(sockfd,buffer,strlen(buffer), 0) <= 0)
		{
			break;
		}
		printf("send: %s\n",buffer);
	}

	close(sockfd);

	return 0;
}
