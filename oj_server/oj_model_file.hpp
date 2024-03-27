#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <fstream>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_model
{

    using namespace ns_log;
    using namespace ns_util;

    // 题目相关信息
    struct Question
    {
        std::string number; // 编号 unique
        std::string title; // 标题
        std::string star; // 难度
        std::string desc; // 题目描述
        std::string header; // 题目预设代码
        std::string tail; // 测试用例
        int cpuLimit; // 时间限制 (s)
        int memLimit; // 空间限制 (KB)
    };

    const std::string questionListPath = "./questions/questions.list";
    const std::string questionsDirPath = "./questions/";

    class Model
    {
    public:
        Model()
        {
            assert(LoadQuestionList(questionListPath));
        }
        ~Model(){}
        
        /**
         * @brief 加载配置文件
         * 加载配置文件 oj_server/questions/questions.list
         */
        bool LoadQuestionList(const std::string &questioList)
        {
            std::ifstream in(questionListPath);
            if(!in.is_open())
            {
                LOG(FATAL) << "题库加载失败\n";
                return false;
            }

            std::string line;
            while(getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, " ");
                if(tokens.size() < 5)
                {
                    LOG(WARNING) << "个别题目配置出现错误\n";
                    continue;
                }
                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpuLimit = atoi(tokens[3].c_str());
                q.memLimit = atoi(tokens[4].c_str());

                std::string numberQuestionPath = questionsDirPath + q.number + "/";
                FileUtil::ReadFile(numberQuestionPath + "desc.txt", &(q.desc), true);
                FileUtil::ReadFile(numberQuestionPath + "header.cpp", &(q.header), true);
                FileUtil::ReadFile(numberQuestionPath + "tail.cpp", &(q.tail), true);

                _questionsMap.insert({q.number, q});
            }

            LOG(INFO) << "加载题库成功\n";
            in.close();
            return true;
        }

        /**
         * @brief 获取题目列表
         * 
         */
        bool GetAllQuestions(std::vector<Question> *out)
        {
            if(_questionsMap.empty())
            {
                LOG(ERROR) << "获取题库失败\n";
                return false;
            }
            for(const auto &q : _questionsMap)
            {
                out->push_back(q.second);
            }
            return true;
        }

        /**
         * @brief 获取指定题目
         * 
         */
        bool GetOneQuestion(const std::string &nummber, Question *que)
        {
            const auto &iter = _questionsMap.find(nummber);
            if(iter == _questionsMap.end())
            {
                LOG(ERROR) << "获取题目失败: " << nummber << std::endl; 
                return false;
            }
            (*que) = iter->second;
            return true;
        }
    private:
        std::unordered_map<std::string, Question> _questionsMap; // 编号->信息
    };
}