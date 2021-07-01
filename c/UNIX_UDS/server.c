#include "UNIX_Domain_Socket.h"

int main(int argc, char *argv[])
{
	char rbuf[1024];
	char sbuf[1024];
	uid_t *puid = NULL;
	const char pathname[] = "/home/changun/APUEdemo/c/UNIX_UDS/ufile.socket";
	int listenfd = serv_listen(pathname);
	int connfd = serv_accept(listenfd, puid);
	printf("connfd == %d\n", connfd);

	memset(rbuf, 0, sizeof(rbuf));
	read(connfd, rbuf, sizeof(rbuf));
	printf("serverread:%s\n", rbuf);

	memset(sbuf, 0, sizeof(sbuf));
	sprintf(sbuf, "fileserver===%s", rbuf);
	write(connfd, sbuf, sizeof(sbuf));
	printf("serverwrite:%s\n", sbuf);

	return 0;
}
