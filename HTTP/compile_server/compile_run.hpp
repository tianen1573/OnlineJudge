#pragma once

#include <jsoncpp/json/json.h>

#include "./compiler.hpp"
#include "./runner.hpp"

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun
    {

    public:
        /**
         * @brief 
         * -1 用户代码为空
         * -2 系统将代码写入文件失败
         * -3 代码文件编译失败
         * -4 代码运行前失败
         * =0 代码运行成功
         * >0 代码运行中出错，值为信号值
         */
        static std::string CodeToDEsc(int code, const std::string &fileName)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "用户提交代码为空";
                break;
            case -2:
                desc = "未知错误：-2";
                break;
            case -3:
                if(!FileUtil::ReadFile(PathUtil::BuildCompilerError(fileName), &desc, true))
                {
                    desc = "代码编译失败";
                }
                break;
            case -4:
                desc = "未知错误：-4";
                break;
            case SIGXCPU:
                desc = "时间超出限制";
                break;
            case SIGABRT:
                desc = "内存超出限制";
                break;
            case SIGKILL:
                desc = "资源超出限制";
                break;
            case SIGFPE:
                desc = "除零/浮点数溢出";
                break;
            default:
                desc = "未知错误：" + std::to_string(code);
                break;
            }

            return desc;
        }

        /**
         * @brief 
         * 
         */
        static void RemoveTempFile(const std::string &fileName)
        {
            // cpp,compiler_error,stdin,stdout,stderr
            std::string _src = PathUtil::BuildSrc(fileName);
            if(FileUtil::IsFileExists(_src))
                unlink(_src.c_str());

            std::string _compiler_error = PathUtil::BuildCompilerError(fileName);
            if(FileUtil::IsFileExists(_compiler_error))
                unlink(_compiler_error.c_str());

            std::string _stdin = PathUtil::BuildStdin(fileName);
            if(FileUtil::IsFileExists(_stdin))
                unlink(_stdin.c_str());

            std::string _stdout = PathUtil::BuildStdout(fileName);
            if(FileUtil::IsFileExists(_stdout))
                unlink(_stdout.c_str());

            std::string _stderr = PathUtil::BuildStderr(fileName);
            if(FileUtil::IsFileExists(_stderr))
                unlink(_stderr.c_str());

            std::string _exe = PathUtil::BuildExe(fileName);
            if(FileUtil::IsFileExists(_exe))
                unlink(_exe.c_str());
        }

        /**
         * @brief 
         * 
         * inJson：输入参数
         * outJson：输出参数
         * 
         * 输入/inJson结构：
         *      code：用户提供的代码
         *      inout：用户自测样例(不处理)
         *      cpuLimit：时间要求
         *      memLimit：空间要求
         * 输出/outJson结构:
         *      status：状态码
         *      reason：状态码描述
         *      stdout：程序运行完成的-*结果(选填)
         *      stderr：程序运行失败的错误结果
         */
        static void Start(const std::string &inJson, std::string *outJson)
        {
            // 输入字符串->结构化数据
            Json::Value inValue;
            Json::Reader reader;
            reader.parse(inJson, inValue);
            std::string code = inValue["code"].asString();
            std::string input = inValue["input"].asString();
            int cpuLimit = inValue["cpuLimit"].asInt();
            int memLimit = inValue["memLimit"].asInt();

            // 输出结构
            Json::Value outValue;

            int statusCode;
            std::string fileName;
            int runRetVal;

            if (code.empty())
            {
                // 用户提交的代码为空
                statusCode = -1; // 代码为空
                goto END;
            }
            // 将code写到临时源文件中
            // 形成唯一的文件名，无目录无后缀，毫秒级时间戳+原子性递增唯一值，后期可用uid
            fileName = FileUtil::UniqFileName();
            if (!FileUtil::WriteFile(PathUtil::BuildSrc(fileName), code))
            {
                // 写入失败
                statusCode = -2; // 代码写入文件中失败
                goto END;
            }
            // 编译
            if (!Compiler::Compile(fileName))
            {
                // 编译失败
                statusCode = -3; // 代码编译失败，内部错误
                goto END;
            }
            runRetVal = Runner::Run(fileName, cpuLimit, memLimit);
            if (runRetVal < 0)
            {
                // 内部错误
                statusCode = -4; // 运行前失败，内部错误
            }
            else if (runRetVal > 0)
            {
                // 运行失败
                statusCode = runRetVal; // 代码运行时失败
            }
            else
            {
                // 运行成功
                statusCode = 0; // 代码运行完成
            }
        END:
            outValue["code"] = statusCode;
            outValue["reason"] = CodeToDEsc(statusCode, fileName);

            if (0 == statusCode)
            {
                // 代码运行成功有结果
                std::string stdoutVal;
                std::string stderrVal;
                FileUtil::ReadFile(PathUtil::BuildStdout(fileName), &stdoutVal, true);
                FileUtil::ReadFile(PathUtil::BuildStderr(fileName), &stderrVal, true);
                outValue["stdout"] = stdoutVal;
                outValue["stderr"] = stderrVal;
            }

            Json::StyledWriter writer;
            *outJson = writer.write(outValue);

            RemoveTempFile(fileName);
        }
    };

}
