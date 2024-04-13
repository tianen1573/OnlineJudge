#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <atomic>
#include <sys/time.h>

#include <boost/algorithm/string.hpp>

namespace ns_util
{

    const std::string temp_path = "./temp/"; // 临时目录路径

    class StringUtil
    {
    public:
        /**
         * @brief 按要求切割字符串，并将内容保存至指定未知
         * src: 源字符串
         * out: 结果保存未知
         * sep: 切割字符
         * 
         */
        static void SplitString(const std::string &src,  std::vector<std::string> *target, const std::string &sep)
        {
            // boost::is_any_of ： 全匹配
            // boost::token_compress_on ： 分隔符之间无数据不存储
            boost::split((*target), src, boost::is_any_of(sep), boost::token_compress_on);
        }
    };

    class TimeUtil
    {
    public:
        static std::string GetTimeStampToMS() // 毫米级时间戳
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            return (std::to_string(tv.tv_sec * 1000 + tv.tv_usec / 1000));
        }
        static time_t GetTimeStampToSecond()
        { // 时间戳
            time_t res;
            res = time(nullptr);
            // std::cout << res << std::endl;
            return res;
        }
        static std::string GetTimeStampToStr()
        { // 制式时间
            struct timeval tv;
            gettimeofday(&tv, nullptr);

            struct tm *tm;
            tm = localtime(&(tv.tv_sec)); // 返回的全局变量区域的指针，不需要释放
            char timeBuff[100];
            strftime(timeBuff, sizeof timeBuff, "%Y-%m-%d || %H:%M:%S", tm); // 转换成制式时间
            // std::cout << timeBuff << std::endl;
            return timeBuff;
        }
    };

    class PathUtil
    {
        /*
        * codeFile -> ./temp/codeFile.cpp
        * codeFile -> ./temp/codeFile.exe
        * codeFile -> ./temp/codeFile.stderr
        * codeFile -> ./temp/codeFile.compiler_error
        */
    public:
        /*
        * 拼接字符串
        */
        static std::string AddSuffix(const std::string &targetFile, const std::string &suff)
        {
            std::string filePath = temp_path;
            filePath += targetFile;
            filePath += suff;
            return filePath;
        }

        // 编译

        /*
        * 构建 源文件的完整路径+后缀
        * targetFile -> targetFile.cpp
        */
        static std::string BuildSrc(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".cpp");
        }
        /*
        * 构建 可执行程序的完整路径+后缀
        * targetFile -> targetFile.exe
        */
        static std::string BuildExe(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".exe");
        }
        /*
        * 构建 编译错误信息的完整路径+后缀
        * targetFile -> targetFile.compiler_error
        */
        static std::string BuildCompilerError(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".compiler_error");
        }

        // 运行

        /*
        * 构建程序运行所需的输入信息的路径
        */
        static std::string BuildStdin(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".stdin");
        }
        /*
        * 构建程序运行的输出信息的路径
        */
        static std::string BuildStdout(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".stdout");
        }
        /*
        * 构建程序运行失败时的路径
        */
        static std::string BuildStderr(const std::string &targetFile)
        {
            return AddSuffix(targetFile, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        /**
         * @brief 判断文件是否存在
         * 
         */
        static bool IsFileExists(const std::string &filePath)
        {
            // 文件是否能打开
            // 是否能够获取文件属性

            struct stat st;
            if (0 == stat(filePath.c_str(), &st))
            {
                // 获取文件属性成功，并保存在st中
                return true;
            }

            return false;
        }

        /**
         * @brief 生成唯一的文件名
         * 形成唯一的文件名，无目录无后缀
         * 毫秒级时间戳+原子性递增唯一值，后期可用uid
         * 
         * 原子性递增值：加锁 或者 atomic
         */
        static std::string UniqFileName()
        {
            static std::atomic_uint id(0);
            ++id;
            std::string ms = TimeUtil::GetTimeStampToMS();
            std::string virUniqId = std::to_string(id);
            return ms + '.' + virUniqId;
        }

        /**
         * @brief 写文件
         * 
         */
        static bool WriteFile(const std::string &targetFile, const std::string &src)
        {
            std::ofstream fout(targetFile);
            if (!fout.is_open())
            {
                return false; // 文件打开失败
            }
            fout.write(src.c_str(), src.size());
            fout.close();
            return true;
        }

        /**
         * @brief 读文件
         * 
         */
        static bool ReadFile(const std::string &targetFile, std::string *desc, bool keepN = false)
        {

            std::ifstream fin(targetFile);
            if (!fin.is_open())
            {
                return false; // 文件打开失败
            }
            else
            {
                (*desc).clear();
                std::string line;
                // getline：不保存行分隔符，并将内容强制转换
                while (getline(fin, line))
                {
                    (*desc) += (line + (keepN ? "\n" : ""));
                }
            }

            return true;
        }
    };
}