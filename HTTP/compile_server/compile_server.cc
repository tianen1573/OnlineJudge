#include "./compile_run.hpp"
#include "../comm/httplib.h"

#include <iostream>
#include <string>

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr << "Usage: "
              << "\n\t" << proc << " port" << std::endl;
}

// 外界提供端口 ./compile_server port
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        return -1;
    }

    Server svr;

    // svr.set_base_dir("./wwwroot");
    svr.set_keep_alive_max_count(10); // Default is 5
    svr.set_keep_alive_timeout(10);  // Default is 5

    svr.Post("/compile_and_run", [](const Request &req, Response &resp)
             {
                 // 正文：inJson
                 // 结果: outJson
                 std::string inJson = req.body;
                 std::string outJson;
                 if (!inJson.empty())
                 {
                     CompileAndRun::Start(inJson, &outJson);
                     resp.set_content(outJson, "application/json; charset=utf-8");
                 }
             });

    LOG(INFO) << "编译服务启动成功，端口号为：" << argv[1] << std::endl;
    svr.listen("0.0.0.0", atoi(argv[1])); // 启动http服务

    return 0;
}