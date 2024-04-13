
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    int ret = Solution().sum(1, 1);
    if(2 == ret)
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
    if(0 == ret)
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

    Test1();
    Test2();

    return 0;
}