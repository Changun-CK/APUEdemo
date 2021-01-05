#include "UNIX_Domain_Socket.h"

int main(int argc, char *argv[])
{
	char rbuf[1024];
	char sbuf[1024];
	const char pathname[] = "/home/changun/UNIX/ufile.socket";
	int clientfd = cli_conn(pathname);
	printf("clientfd == %d\n", clientfd);

	memset(sbuf, 0, sizeof(sbuf));
	sprintf(sbuf, "=%s", "hello, world");
	write(clientfd, sbuf, sizeof(sbuf));
	printf("clientwrite:%s\n", sbuf);

	memset(rbuf, 0, sizeof(rbuf));
	sleep(3);
	read(clientfd, rbuf, sizeof(rbuf));
	printf("clientread:%s\n", rbuf);

	return 0;
}
