#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <ctemplate/template.h>

#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "./oj_model_mysql.hpp"
// #include "./oj_model_file.hpp"

namespace ns_view
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;

    const std::string templateHtmlDirPath = "./template_html/";

    class View
    {
    public:
        View() {}
        ~View() {}

        /**
         * @brief 根据难度设置标签颜色
         * 
         */
        std::string StarToColor(const std::string &star)
        {
            const std::unordered_map<std::string, std::string> sc = {
                {"简单", "<span style = \"color:green\">"},
                {"中等", "<span style = \"color:yellow\">"},
                {"困难", "<span style = \"color:red\">"}
                };
            
            return sc.at(star);
        }

        /**
         * @brief 渲染题目列表网页
         * allQuestions：所有题目的信息
         * outHtml：输出网页
         */
        void AllQuestionsExpandHtml(const std::vector<struct Question> &allQuestions, std::string *outHtml)
        {
            // 题目编号 题目标题 题目难度
            // 1. 形成路径
            std::string srcHtml = templateHtmlDirPath + "all_questions.html";
            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("allQuestions");
            for (const auto &ques : allQuestions)
            {
                // 往根字典中添加子字典
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("questionsList");
                sub->SetValue("number", ques.number);
                sub->SetValue("title", ques.title);
                sub->SetValue("star", StarToColor(ques.star) + ques.star + "</span>");
            }
            // 3. 获取待渲染网页
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(srcHtml, ctemplate::DO_NOT_STRIP);
            // 4. 渲染网页
            tpl->Expand(outHtml, &root);
        }

        /**
         * @brief 渲染指定题目网页
         * ques：指定题目的信息
         * outHtml：输出网页
         */
        void OneQuestionExpandHtml(const struct Question &ques, std::string *outHtml)
        {
            // 1. 形成路径
            std::string srcHtml = templateHtmlDirPath + "one_question.html";
            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("oneQuestion");
            root.SetValue("number", ques.number);
            root.SetValue("title", ques.title);
            root.SetValue("star", StarToColor(ques.star) + ques.star + "</span>");
            root.SetValue("desc", ques.desc);
            root.SetValue("header", ques.header);
            // 3. 获取待渲染网页
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(srcHtml, ctemplate::DO_NOT_STRIP);
            // 4. 渲染网页
            tpl->Expand(outHtml, &root);
        }
    };
}