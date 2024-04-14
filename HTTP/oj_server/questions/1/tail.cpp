
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif
#include <csignal>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

void handleAlarm(int signum)
{
    exit(SIGALRM);
}
void Test1()
{
    int ret = Solution().sum(1, 1);
    if (2 == ret)
    {
        std::cout << "用例1通过，测试用例：1, 1" << std::endl;
    }
    else
    {
        std::cout << "用例1未通过，测试用例：1, 1" << std::endl;
    }
}
void Test2()
{
    int ret = Solution().sum(-1, 1);
    if (0 == ret)
    {
        std::cout << "用例2通过，测试用例：-1, 1" << std::endl;
    }
    else
    {
        std::cout << "用例2未通过，测试用例：-1, 1" << std::endl;
    }
}

int main()
{
    // 设置信号处理函数
    signal(SIGALRM, handleAlarm);
    // 设置 1*2 秒的定时器
    alarm(2);
    Test1();
    Test2();

    return 0;
}