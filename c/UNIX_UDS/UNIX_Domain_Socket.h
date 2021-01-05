#ifndef _UNIX_DOMAIN_SOCKET_H
#define _UNIX_DOMAIN_SOCKET_H

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>

#define QLEN 10
#define STALE 30 /* client's name can't be older than this (sec) */
#define CLI_PATH "/var/tmp"
#define CLI_PERM S_IRWXU

/*
 * create a server endpoint of a connection.
 * Returns fd if all OK, <0 on error.
 */
int serv_listen(const char *name);

/*
 * Wait for a client connection to arrive, and accept it.
 * We also obtain the client's user ID from the pathname
 * that it must bind before calling us.
 * Returns new fd if all OK, <0 on error.
 */
int serv_accept(int listenfd, uid_t *uidptr);

/*
 * Create a client endpoint and connect to a server.
 * Returns fd if all OK, <0 on error.
 */
int cli_conn(const char *name);

#endif
