
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
    Solution t;
    bool ret = t.isPalindromeString("aaabbaaa");
    if(true == ret)
    {
        std::cout << "用例1<span style = \"color:#2DB55D\">通过</span>，测试用例：aaabbaaa<br>" << std::endl;
    }
    else
    {
        std::cout << "用例1<span style = \"color:#EF4743\">未通过</span>，测试用例：aaabbaaa<br>" << std::endl;
    }
}
void Test2()
{
    Solution t;
    bool ret = t.isPalindromeString("abaa");
    if(false == ret)
    {
        std::cout << "用例2<span style = \"color:#2DB55D\">通过</span>，测试用例：abaa<br>" << std::endl;
    }
    else
    {
        std::cout << "用例2<span style = \"color:#EF4743\">未通过</span>，测试用例：abaa<br>" << std::endl;
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