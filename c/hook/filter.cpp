/**
 * 本文利用hook主要实现网络监控
 * 作者: changun, 时间: 2021-07-03
 **/

#include "filter.h"

namespace filter
{

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(connect)

bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

void hook_init()
{
    static bool is_inited = false;
    if (is_inited)
    {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

struct _HookIniter
{
    _HookIniter()
    {
        hook_init();
    }
};

static _HookIniter s_hook_initer;

}

#ifdef __cplusplus
extern "C" {
#endif

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX


// 判断接入的网络是否为过滤IP，如果是则提前返回
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    // 如果不设置hook，那么直接放行
    if(!filter::t_hook_enable)
    {
        return connect_f(sockfd, addr, addrlen);
    }

    char ip[128]; memset(ip, 0, sizeof(ip)); // ip地址
    int port = -1;                           // 端口号

    if (AF_INET == addr->sa_family)
    {
        struct sockaddr_in *sa4 = (struct sockaddr_in *)addr;
        inet_ntop(AF_INET, (void *)(struct sockaddr *)&sa4->sin_addr, ip, sizeof(ip));
        port = ntohs(sa4->sin_port);
        printf("\nAF_INET IP===%s:%d\n", ip, port);
    }

    if (0 == strcmp(ip, URL))
    {
        printf("\n===%s netfilter...connect failed!\n", ip);
        return -1;
    }

    printf("\nPID:%d, socket:%d, %s Successfully connected!\n", getpid(), sockfd, ip);
    return connect_f(sockfd, addr, addrlen);
}

#ifdef __cplusplus
}
#endif
