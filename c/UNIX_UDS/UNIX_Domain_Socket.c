#include "UNIX_Domain_Socket.h"

int serv_listen(const char *name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	if (strlen(name) >= sizeof(un.sun_path))
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* create a UNIX domain stream socket */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return -2;

	unlink(name); /* in case it already exists */

	/* fill in socket address structure */
	memset(&un, 0, sizeof(struct sockaddr_un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	// offsetof 是结构某成员相对结构开头的偏移量
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

	/* bind the name to the descriptor */
	if (bind(fd, (struct sockaddr *)&un, len) < 0)
	{
		rval = -3;
		goto errout;
	}

	if (listen(fd, QLEN) < 0) /* tell kernel we're a server */
	{
		rval = -4;
		goto errout;
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return (rval);
}

int serv_accept(int listenfd, uid_t *uidptr)
{
	int                 clifd, err, rval;
	socklen_t           len;
	time_t              staletime;
	struct sockaddr_un  un;
	struct stat         statbuf;
	char                *name;

	/* allocate enough space for longest name plus terminating null */
	if ((name = malloc(sizeof(un.sun_path) + 1)) == NULL)
		return -1;

	len = sizeof(struct sockaddr_un);
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
	{
		free(name);
		return -2; /* often errno=EINTR, if signal caught */
	}

	/* obtain the client's uid from its calling address */
	len = len - offsetof(struct sockaddr_un, sun_path); /* len of pathname */
	memcpy(name, un.sun_path, len);
	name[len] = 0;
	if (stat(name, &statbuf) < 0)
	{
		rval = -3;
		goto errout;
	}

#ifdef   S_ISSOCK         /* not defined for SVR4 */
	if (S_ISSOCK(statbuf.st_mode) == 0)
	{
		rval = -4;
		goto errout;
	}
#endif

	if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) ||
		(statbuf.st_mode & S_IRWXU) != S_IRWXU)
	{
		rval = -5;
		goto errout;
	}

	staletime = time(0) - STALE;
	if (statbuf.st_atime < staletime ||
		statbuf.st_ctime < staletime ||
		statbuf.st_mtime < staletime)
	{
		rval = -6;
		goto errout;
	}

	if (uidptr != NULL)
	{
		*uidptr = statbuf.st_uid; /* return uid of caller */
	}

	unlink(name);
	free(name);
	return clifd;

errout:
	err = errno;
	close(clifd);
	free(name);
	errno = err;
	return (rval);
}

int cli_conn(const char *name)
{
	int                 fd, len, err, rval;
	struct sockaddr_un  un, sun;
	int                 do_unlink = 0;

	if (strlen(name) >= sizeof(un.sun_path))
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* create a UNIX domain stream socket */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return -1;
	
	/* fill socket address structure with out address */
	memset(&un, 0, sizeof(struct sockaddr_un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s/%05ld", CLI_PATH, (long)getpid());
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

	unlink(un.sun_path);  /* in case it already exists */
	if (bind(fd, (struct sockaddr *)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}

	if (chmod(un.sun_path, CLI_PERM) < 0)
	{
		rval = -3;
		do_unlink = 1;
		goto errout;
	}

	/* fill socket address structure with server's address */
	memset(&sun, 0, sizeof(struct sockaddr_un));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (connect(fd, (struct sockaddr *)&sun, len) < 0)
	{
		rval = -4;
		do_unlink = 1;
		goto errout;
	}

	return fd;

errout:
	err =errno;
	close(fd);
	if (do_unlink)
		unlink(un.sun_path);

	errno = err;
	return (rval);
}
