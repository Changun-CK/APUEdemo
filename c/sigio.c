#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int sockfd = 0;

void do_sigio(int sig)
{
	struct sockaddr_in cli_addr;
	int client = sizeof(struct sockaddr_in);
	int clifd = 0;
#if 0
	clifd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

	char buffer[256] = { 0 };
	printf("Listen Message: %s\n", buffer);

	int slen = write(clifd, buffer, len);
#else
	char buffer[256] = { 0 };
	int len = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr*)&cli_addr, (socklen_t*)&client);
	printf("Listen Message: %s\n", buffer);

	int slen = sendto(sockfd, buffer, len, 0, (struct sockaddr*)&cli_addr, client);
#endif
}

int main()
{
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // udp协议

	struct sigaction sigio_action;
	sigio_action.sa_flags = 0;
	sigio_action.sa_handler = do_sigio;    // 处理函数
	sigaction(SIGIO, &sigio_action, NULL); // 传递

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5050);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// 设置将接收SIGIO和SIGURG信号的进程id或进程组id，进程组id通过提供负值的arg来说明(arg绝对值的一个进程组ID)，否则arg将被认为是进程id
	fcntl(sockfd, F_SETOWN, getpid()); // 设置异步I/O的所有权

	bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	while (1) sleep(1);

	close(sockfd);

	return 0;
}
