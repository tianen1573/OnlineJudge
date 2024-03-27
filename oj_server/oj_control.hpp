#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include <mutex>
#include <assert.h>
// #include <pthread.h>

#include "./oj_model_mysql.hpp"
// #include "./oj_model_file.hpp"
#include "./oj_view.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "../comm/httplib.h"

namespace ns_control
{

    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;

    const std::string banCodePath = "questions/banCode.cpp";
    const std::string serviceMachinePath = "conf/service_machine.conf"; // oj_server/conf/service_machine.conf
    class Machine                                                         // 主机
    {
    public:
        Machine()
            : _ip(""), _port(0), _load(0), _mtx(nullptr)
        {
        }
        ~Machine() {}
        void SetPort(int port)
        {
            _port = port;
        }
        int GetPort()
        {
            return _port;
        }
        void SetIp(const std::string &ip)
        {
            _ip = ip;
        }
        std::string GetIp()
        {
            return _ip;
        }
        void SetLoad(uint64_t load)
        {
            _load = load;
        }
        uint64_t GetLoad()
        {
            uint64_t load = 0;
            if (_mtx)
                _mtx->lock();

            load = _load;

            if (_mtx)
                _mtx->unlock();

            return load;
        }
        void SetMutex(std::mutex *mtx)
        {
            _mtx = mtx;
        }

        /**
         * @brief 增加主机负载
         * 
         */
        void IncLoad()
        {
            if (_mtx)
                _mtx->lock();

            ++_load;

            if (_mtx)
                _mtx->unlock();
        }

        /**
         * @brief 减少主机负载
         * 
         */
        void DecLoad()
        {
            if (_mtx)
                _mtx->lock();

            --_load;

            if (_mtx)
                _mtx->unlock();
        }
        /**
         * @brief 重置主机负载情况
         * 
         */
        void ResetLoad()
        {
            if(_mtx) _mtx->lock();

            _load = 0;

            if(_mtx) _mtx->unlock();
        }

    private:
        std::string _ip;
        int _port;
        uint64_t _load;
        std::mutex *_mtx;
    };

    class LoadBlance // 负载均衡
    {
    public:
        LoadBlance()
        {
            assert(LoadConf(serviceMachinePath));
        }
        ~LoadBlance() {}

        /**
         * @brief 加载所有主机
         * serviceMachineConf: 主机配置信息文件路径
         */
        bool LoadConf(const std::string &serviceMachineConf)
        {
            std::ifstream in(serviceMachineConf);
            if (!in.is_open())
            {
                LOG(FATAL) << "主机配置文件加载失败\n";
                return false;
            }
            std::string line;
            while (getline(in, line))
            {
                std::vector<std::string> strs;
                StringUtil::SplitString(line, &strs, ":");
                if (2 != strs.size())
                {
                    LOG(WARING) << "主机信息错误：" << line << std::endl;
                    continue;
                }

                Machine m;
                m.SetIp(strs[0]);
                m.SetPort(atoi(strs[1].c_str()));
                m.SetLoad(0);
                m.SetMutex(new std::mutex());

                _online.push_back(_machines.size());
                _machines.push_back(m);
            }

            in.close();
            LOG(INFO) << "主机配置文件加载成功\n";
            return true;
        }

        /**
         * @brief 选择负载最低的主机提供编译运行服务 
         * *id： 选择的主机的id
         * *m：选择的主机的对象的地址
         * return：真为成功
         * 
         * 1. 随机数+hash
         * 2. 轮询+hash
         */
        bool SmartChoice(int *id, Machine **m)
        {
            _mtx.lock();

            int onlineNum = _online.size();
            if (0 == onlineNum) // 无在线主机
            {
                _mtx.unlock();
                LOG(FATAL) << "所有主机全部离线，请注意，请注意！\n";
                return false;
            }

            // 轮询查找负载最小的机器
            uint64_t minLoad = _machines[_online[0]].GetLoad();
            *id = _online[0];
            *m = &_machines[_online[0]];
            for (int i = 1; i < onlineNum; i++)
            {
                uint64_t currLoad = _machines[_online[i]].GetLoad();
                if (minLoad > currLoad)
                {
                    minLoad = currLoad;
                    *id = _online[i];
                    *m = &_machines[_online[i]];
                }
            }

            _mtx.unlock();

            return true;
        }

        /**
         * @brief 离线指定主机
         * 
         */
        void OfflineMachine(int machineId)
        {
            _mtx.lock();

            for (auto iter = _online.begin(); iter != _online.end(); iter++)
            {
                if (*iter == machineId)
                {
                    _machines[machineId].ResetLoad();
                    _online.erase(iter);
                    _offline.push_back(machineId);
                    break; // erase之后，迭代器不再++，无迭代器失效问题
                }
            }

            _mtx.unlock();
        }

        /**
         * @brief 上线指定主机
         * 当所有主机离线时，上线所有主机
         */
        void OnlineMachine()
        {
            _mtx.lock();

            _online.insert(_online.end(), _offline.begin(), _offline.end());
            _offline.erase(_offline.begin(), _offline.end());

            _mtx.unlock();

            LOG(INFO) << "重新上线所有主机!\n";
        }

        /**
         * @brief 显示在线主机，for test
         * 
         */
        void ShowMachines()
        {
            _mtx.lock();
            std::cout << "在线主机列表：";
            for (auto &id : _online)
            {
                std::cout << id << " ";
            }
            std::cout << "\n离线主机列表：";
            for (auto &id : _offline)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            _mtx.unlock();
        }

    private:
        std::vector<Machine> _machines; // 下标即代表主机id
        std::vector<int> _online;       // 在线主机ID
        std::vector<int> _offline;      // 离线主机ID
        std::mutex _mtx;
    };

    class Control
    {
    public:
        Control() 
        {
            assert(FileUtil::ReadFile(banCodePath, &_banCode, true));
        }
        ~Control() {}

        /**
         * @brief 构建题目列表html
         * html：输出型参数
         */
        bool AllQuestionsHtml(std::string *html)
        {
            std::vector<struct Question> allQues;
            if (!_model.GetAllQuestions(&allQues))
            {
                LOG(ERROR) << "获取题库失败\n";
                *html = "获取题目列表失败，请刷新！";
                return false;
            }

            std::sort(allQues.begin(), allQues.end(), [](const struct Question &q1, const struct Question &q2){
                return std::atoi(q1.number.c_str()) < std::atoi(q2.number.c_str());
            });
            _view.AllQuestionsExpandHtml(allQues, html);

            return true;
        }

        /**
         * @brief 获取指定题目的html
         * 
         */
        bool QuestionHtml(const std::string &number, std::string *html)
        {
            struct Question ques;
            if (!_model.GetOneQuestion(number, &ques))
            {
                LOG(ERROR) << "获取题目失败：" << number << std::endl;
                *html = "指定题目不存在";
                return false;
            }

            _view.OneQuestionExpandHtml(ques, html);

            return true;
        }

        /**
         * @brief 
         * 
        * 输入/inJson结构：
         *      code：用户提供的代码
         *      input：用户自测样例(不处理)
         * 输出/outJson结构:
         *      status：状态码
         *      reason：状态码描述
         *      stdout：程序运行完成的-*结果(选填)
         *      stderr：程序运行失败的错误结果
         */
        void Judge(const std::string &number, const std::string &inJson, std::string *outJson)
        {
            // 0. 获取题目信息
            struct Question ques;
            _model.GetOneQuestion(number, &ques);
            // 1. 反序列化
            Json::Reader reader;
            Json::Value inVal;
            reader.parse(inJson, inVal);
            // 2. 拼接代码
            Json::Value compileVal;
            compileVal["input"] = inVal["input"].asString();
            compileVal["code"] = _banCode + inVal["code"].asString() + ques.tail;
            compileVal["cpuLimit"] = ques.cpuLimit;
            compileVal["memLimit"] = ques.memLimit;
            Json::FastWriter writer;
            std::string compileJson = writer.write(compileVal);
            // 3. 负载均衡
            // 一直选择，直到主机可用且功能正常，否则代表全部挂掉
            while (true)
            {
                int id = 0;
                Machine *m = nullptr;

                if (!_loadBlance.SmartChoice(&id, &m))
                {
                    break;
                }
                LOG(INFO) << "主机选择成功，主机：" << id << ":" << m->GetIp() << ":" << m->GetPort() << "，负载：" << m->GetLoad() << std::endl;
                // 4. http请求
                httplib::Client cli(m->GetIp(), m->GetPort());
                m->IncLoad();
                if (auto res = cli.Post("/compile_and_run", compileJson, "application/json"))
                {
                    // 5. 返回结果
                    if (res->status == 200)
                    {
                        *outJson = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求编译运行服务成功\n";
                        break;
                    }
                    m->DecLoad();
                }
                else
                {
                    LOG(ERROR) << "当前主机已离线，主机ID：" << id << "，主机：" << m->GetIp() << ":" << m->GetPort() << std::endl;
                    _loadBlance.OfflineMachine(id);
                    _loadBlance.ShowMachines();
                }
            }
        }

        /**
         * @brief 上线所有主机
         * 
         */
        void RecoveryMachine()
        {
            _loadBlance.OnlineMachine();
        }
    private:
        Model _model;
        View _view;
        LoadBlance _loadBlance;
        std::string _banCode;
    };
}