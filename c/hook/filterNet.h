#ifndef __FILTER_NET_H__
#define __FILTER_NET_H__

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <dlfcn.h>

#define HOOK_SYS_FUNC(name) if( !g_sys_##name##_func ) { g_sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT,#name); }

namespace filterNet
{

bool is_hook_enable();
void set_hook_enable(bool flag);

};

char fip[128] = { 0 };     // 监控的IP, 禁止连接该IP
void setFilterIp(const char *ip)
{
    strcpy(fip, ip);
}

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*connect_pfn_t)(int socket, const struct sockaddr *address, socklen_t address_len);
static connect_pfn_t g_sys_connect_func = (connect_pfn_t)dlsym(RTLD_NEXT,"connect");

#ifdef __cplusplus
}
#endif

#endif
