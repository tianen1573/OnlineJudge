#include <iostream>
#include <signal.h>

#include "../comm/httplib.h"
#include "./oj_control.hpp"

using namespace httplib;
using namespace ns_control;

static Control *pCtrl = nullptr;

void Recovery(int signo)
{
    pCtrl->RecoveryMachine();
}

int main()
{

    signal(SIGQUIT, Recovery);

    // 用户请求的服务路由功能
    Server svr;
    Control ctrl;
    pCtrl = &ctrl;

    // 1. 获取所有的题目列表
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp) {
        std::string html;
        ctrl.AllQuestionsHtml(&html);
        resp.set_content(html, "text/html;charset=utf-8");
    });

    // 2. 根据题目编号，获取题目内容
    // \d+: 正则表达式
    // R"()": 保持字符串内容的原貌，不用做转义
    svr.Get(R"(/questions/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string html;
        std::string number = req.matches[1]; // 能够获取到正则表达式\d+的内容
        ctrl.QuestionHtml(number, &html);
        resp.set_content(html, "text/html;charset=utf-8");
    });

    // 3. 提交代码，使用判题功能
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1]; // 能够获取到正则表达式\d+的内容
        std::string resultJson;
        std::cout << req.body << std::endl;
        ctrl.Judge(number, req.body, &resultJson);
        resp.set_content(resultJson, "application/json; charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");

    svr.listen("0.0.0.0", 8084);

    return 0;
}