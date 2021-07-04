#include "filterNet.h"

namespace filterNet
{

static thread_local bool t_hook_enable = false;

bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

}

#ifdef __cplusplus
extern "C" {
#endif

int connect(int fd, const struct sockaddr *address, socklen_t address_len)
{
    HOOK_SYS_FUNC(connect);
    if(!filterNet::t_hook_enable)
    {
        printf("no hook.\n");
        return g_sys_connect_func(fd, address, address_len);
    }

    // hook
    char ip[64];
    int port = -1;
    if (AF_INET == address->sa_family)
    {
        struct sockaddr_in *sa = (struct sockaddr_in *)address;
        inet_ntop(AF_INET, (void *)(struct sockaddr *)&sa->sin_addr, ip, sizeof(ip));
        port = ntohs(sa->sin_port);
        printf("\nAF_INET IP===%s:%d\n", ip, port);
    }

    if (0 == strcmp(ip, fip))
    {
        printf("%s netfilter...connect failed!\n", ip);
        return -1;
    }

    printf("\nPID:%d, socket:%d, %s Successfully connected!\n", getpid(), fd, ip);
    return g_sys_connect_func(fd, address, address_len);
}

#ifdef __cplusplus
}
#endif
