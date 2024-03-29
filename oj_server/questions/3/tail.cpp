
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1(Solution &t)
{
    bool ret = t.isPrime(1);
    if(false == ret)
    {
        std::cout << "用例1通过，测试用例：1" << std::endl;
    }
    else
    {
        std::cout << "用例1未通过，测试用例：1" << std::endl;
    }
}
void Test2(Solution &t)
{
    bool ret = t.isPrime(3);
    if(true == ret)
    {
        std::cout << "用例2通过，测试用例：3" << std::endl;
    }
    else
    {
        std::cout << "用例2未通过，测试用例：3" << std::endl;
    }
}

int main()
{
    Solution t;
    Test1(t);
    Test2(t);

    return 0;
}