
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    Solution t;
    bool ret = t.isPalindromeString("aaabbaaa");
    if(true == ret)
    {
        std::cout << "用例1通过，测试用例：aaabbaaa" << std::endl;
    }
    else
    {
        std::cout << "用例1未通过，测试用例：aaabbaaa" << std::endl;
    }
}
void Test2()
{
    Solution t;
    bool ret = t.isPalindromeString("abaa");
    if(false == ret)
    {
        std::cout << "用例2通过，测试用例：abaa" << std::endl;
    }
    else
    {
        std::cout << "用例2未通过，测试用例：abaa" << std::endl;
    }
}

int main()
{
    Test1();
    Test2();

    return 0;
}