#ifndef __FILTER_H__
#define __FILTER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>

const char URL[] = "192.168.130.128";     // 监控的IP, 禁止连接该IP

namespace filter {
    bool is_hook_enable();
    void set_hook_enable(bool flag);
}

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*connect_fun)(int, const struct sockaddr *, socklen_t);
extern connect_fun connect_f;

#ifdef __cplusplus
}
#endif

#endif
