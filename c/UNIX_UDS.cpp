#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[])
{
	int fd, size;
	struct sockaddr_un un;

	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, "foo.socket");
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("UDS create failed");
		exit(1);
	}

	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(fd, (struct sockaddr *)&un, size) < 0)
	{
		perror("UDS bind failed");
		exit(1);
	}

	printf("UNIX domain socket bound\n");
	close(fd);
	remove(un.sun_path);

	return 0;
}
