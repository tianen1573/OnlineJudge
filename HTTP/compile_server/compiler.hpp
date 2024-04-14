/*
* 代码编译模块
*/

#pragma once

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace ns_compiler {

    using namespace ns_util; // 引入路径拼接
    using namespace ns_log; // 日志

    class Compiler {
        public:
            Compiler() {}
            ~Compiler() {}
            /*
            * 输入参数：
            *   codeFile：需要编译的文件的文件名
            * 返回值：
            *   true：编译成功
            *   false：编译失败
            * 
            */
            static bool Compile(const std::string &fileName) {

                // 创建子进程，进行程序替换，提供编译功能
                pid_t childPid = fork();
                if(childPid < 0) { // 失败
                    LOG(ERROR) << "创建子进程失败" << "\n";

                } else if(childPid == 0) { // 子进程

                    // 将标准错误重定向到_stderr
                    umask(0);
                    int fdStderr = open(PathUtil::BuildCompilerError(fileName).c_str(), O_CREAT | O_WRONLY, 0644);
                    if(fdStderr < 0) {
                        LOG(WARNING) << "生成compiler_error文件失败" << "\n";
                        exit(1); // 生成失败会导致run时打开失败
                    }
                    dup2(fdStderr, 2); // 重定向

                    // g++ -o target src -std=c++11
                    // COMPILER_ONLINE 传入宏用于取消包含头文件
                    // -Werror=return-type 强制代码存在return语句,否则报错
                    // -Wfatal-errors 当遇到错误时，停止编译，避免泄露不必要的信息
                    execlp("g++", \
                    "g++", \
                     "-o",  \
                     PathUtil::BuildExe(fileName).c_str(), \
                     PathUtil::BuildSrc(fileName).c_str(), \
                     "-D", \
                     "COMPILER_ONLINE", \
                     "-Werror=return-type", \
                     "-Wfatal-errors", \
                     "-std=c++11", \
                     nullptr);
                    
                    LOG(ERROR) << "启动g++失败，请注意相关参数" << "\n";
                    exit(2); // 程序替换失，没有生成可执行程序
                } else { // 父进程
                    waitpid(childPid, nullptr, 0);

                    // 如果编译成功了，则可执行程序是能够打开的，stat
                    // 或者通过获取子进程/g++的返回值，判断是否编译成功，进程通信
                    if(FileUtil::IsFileExists(PathUtil::BuildExe(fileName))) {
                        LOG(INFO) << "编译成功：" << PathUtil::BuildSrc(fileName) << "\n";
                        return true;
                    }
                }

                LOG("ERROR") << "编译失败" << std::endl;
                return false;
            }
        private:
    };
}