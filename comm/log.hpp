#pragma once

#include <iostream>
#include <string>

#include "./util.hpp"

namespace ns_log {

    using namespace ns_util; // 工具类
    
    
    // #宏参，可以将参数转换成字符串
    #define LOG(level) Log(#level, __FILE__, __LINE__)

    enum {
        INFO,
        DEBUG,
        WARNING, // 警告
        ERROR, // 用户错误
        FATAL // 系统错误
    };

    // 错误等级， 文件名， 代码所在行
    // LOG() << "message"
    inline static std::ostream& Log(const std::string &leavel, const std::string &fileName, const int line) {
        // [日志等级]--[日志文件]--[所在行]--[时间戳]
        std::string message;

        message += '[';
        message += leavel;
        message += "]--";

        message += '[';\
        message += fileName;
        message += "]--";

        message += '[';
        message += std::to_string(line);
        message += "]--";

        message += '[';
        message += TimeUtil::GetTimeStampToStr();
        message += "]  ";

        std::cout << message; // 不使用行刷新, LOG(level) << "message" << endl; 在调用处进行刷新

        return std::cout;
    }
}