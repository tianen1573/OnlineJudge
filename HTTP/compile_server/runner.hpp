#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace ns_runner
{

    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        Runner() {}
        ~Runner() {}

        /*
        * cpu(s), 内存(M)
        */
        static void SetRlimit(rlim_t cpuLimit, rlim_t memLimit)
        {
            // CPU
            struct rlimit cpuR = {cpuLimit, RLIM_INFINITY};
            setrlimit(RLIMIT_CPU, &cpuR);
            // 内存
            struct rlimit memR = {memLimit * 1024 * 1024, RLIM_INFINITY};
            setrlimit(RLIMIT_AS, &memR);
        }

        /*
        * 输入参数：
        *  codeFile：需要编译的文件的文件名
        *  cpuLimit：CPU资源上限 (s)
        *  memLimit：内存资源上限 (KB)
        * 返回值：
        *  = 0 运行成功，结果正确/错误
        *  > 0 运行失败，返回值代表错误信号(触发信号)
        *  < 0 内部错误，程序没运行
        *       -1 无法打开标准文件
        *       -2 子进程创建失败，即程序根本没运行
        *       -3 子进程替换失败，即程序根本没运行
        * 
        * Run只关心代码是否运行成功，不关心结果的对错
        * 结果对错，由其他代码判定
        * 
        * 标准输入：不处理（用户自测样例）<- codeFile.stdin
        * 标准输出：运行结果 -> codeFile.stdout
        * 标准错误：运行时错误信息 -> codeFile.stderr
        * 注意：程序不正常退出时的状态码不在标准错误中，而是在父进程的返回值中
        */
        static int Run(const std::string &codeFile, int cpuLimit, int memLimit)
        {

            // 构建路径
            std::string exePath = PathUtil::BuildExe(codeFile);
            std::string _stdin = PathUtil::BuildStdin(codeFile);
            std::string _stdout = PathUtil::BuildStdout(codeFile);
            std::string _stderr = PathUtil::BuildStderr(codeFile);
            // 打开文件，父进程关闭，子进程使用
            umask(0);
            int _stdinFd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdoutFd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderrFd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0644);
            // 差错处理
            if (_stdinFd < 0 || _stdoutFd < 0 || _stderrFd < 0)
            {
                LOG(ERROR) << "无法为程序打开标准文件\n";
                return -1;
            }
            // 创建子进程，进行进程替换
            pid_t childPid = fork();
            if (childPid < 0) // 失败
            {
                close(_stdinFd);
                close(_stdoutFd);
                close(_stderrFd);
                LOG(ERROR) << "运行时创建子进程失败\n";
                return -2;
            }
            else if (0 == childPid) // 子进程
            {
                // 重定向
                dup2(_stdinFd, 0);
                dup2(_stdoutFd, 1);
                dup2(_stderrFd, 2);
                // 资源约束 时间 1s，空间 1G
                SetRlimit(cpuLimit, memLimit);
                // LOG(INFO) << "SetRlimit\n";
                // path, exe
                execl(exePath.c_str(), exePath.c_str(), nullptr);
                exit(1); // 走到这里代表替换失败
            }
            else // 父进程
            {
                close(_stdinFd);
                close(_stdoutFd);
                close(_stderrFd);

                int status = 0;
                waitpid(childPid, &status, 0); // 阻塞等待
                // 子进程正常结束
                if (WIFEXITED(status))
                {
                    LOG(INFO) << "代码运行完毕，info：" << WEXITSTATUS(status) << std::endl;

                    // 正常结束
                    if(WEXITSTATUS(status) == 0)
                    {
                        return 0;
                    }
                    // 触发SIGALRM，当成触发 SIGXCPU
                    else if(WEXITSTATUS(status) == SIGALRM)
                    {
                        return SIGXCPU;
                    }
                    // 子进程程序替换失败，即用户程序未能运行起来，当成未知错误
                    else if(WEXITSTATUS(status) == 1)
                    {
                        return -3;
                    }
                    else
                    {
                        return WEXITSTATUS(status);
                    }
                }
                else // 非正常结束
                {
                    LOG(INFO) << "代码运行完毕，info：" << WTERMSIG(status) << std::endl;
                    return WTERMSIG(status);
                }
            }
        }
    };
}