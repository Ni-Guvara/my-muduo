#include "CurrentThread.h"

void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}