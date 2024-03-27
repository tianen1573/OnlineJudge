#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <fstream>

#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "./mysqlInclude/mysql.h"


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

    const std::string ojQuestionsTable = "questions";

    class Model
    {
    public:
        Model(){}
        ~Model(){}
        
        /**
         * @brief 查询sql
         * 
         */
        bool QueryMysql(const std::string &sql, std::vector<Question> *out)
        {
            // 创建mysql句柄
            MYSQL *my = mysql_init(nullptr);
            const std::string host = "127.0.0.1";
            const std::string user = "oj_client";
            const std::string passwd = "btjz.88ka.cn";
            const std::string db = "oj";
            const int port = 3306;

            // 连接数据库
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FATAL) << "连接数据库失败！\n";
                return false;
            }
            LOG(INFO) << "连接数据库成功！\n";

            // 同一编码格式
            mysql_set_character_set(my, "utf8");

            // 执行查询语句
            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!\n";
                return false;
            }

            // 提取结果
            MYSQL_RES *res = mysql_store_result(my);

            // 分析结果
            int rows = mysql_num_rows(res);
            int cols = mysql_num_fields(res);
            for(int i = 0; i < rows; ++ i)
            {
                // 提取数据
                MYSQL_ROW curRow= mysql_fetch_row(res);
                struct Question que;
                que.number = curRow[0];
                que.title = curRow[1];
                que.star = curRow[2];
                que.desc = curRow[3];
                que.header = curRow[4];
                que.tail = curRow[5];
                que.cpuLimit = std::atoi(curRow[6]);
                que.memLimit = std::atoi(curRow[7]);
                // 放入out
                out->push_back(que);
            }

            // 释放资源，关闭连接
            mysql_free_result(res);
            mysql_close(my);

            return true;
        }

        /**
         * @brief 获取题目列表
         * 
         */
        bool GetAllQuestions(std::vector<Question> *out)
        {
            std::string sql = "select * from " + ojQuestionsTable;
            return QueryMysql(sql, out);
        }

        /**
         * @brief 获取指定题目
         * 
         */
        bool GetOneQuestion(const std::string &number, Question *que)
        {
            std::string sql = "select * from " + ojQuestionsTable;
            sql += " where number = ";
            sql += number;
            std::vector<Question> resl;
            if(QueryMysql(sql, &resl))
            {
                if(resl.size() == 1)
                {
                    *que = resl[0];
                    return true;
                }
            }
            
            return false;
        }
    };
}