#pragma once

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <thread>
#include "../include/mysqlcppInclude/jdbc/mysql_driver.h"
#include "../include/mysqlcppInclude/jdbc/mysql_connection.h"
#include "../include/mysqlcppInclude/jdbc/cppconn/prepared_statement.h"
#include "../include/mysqlcppInclude/jdbc/cppconn/resultset.h"

class ConnectionPool
{
public:
    /**
     * @brief 懒汉单例
     * 
     */
    static ConnectionPool &getInstance(const std::string &host = "127.0.0.1", const std::string &username = "oj_client",
                                       const std::string &password = "btjz.88ka.cn", const std::string &database = "oj",
                                       int maxConnections = 10, int minConnections = 5, int maxIdleTime = 5)
    {
        // 使用双重检查锁定确保线程安全
        if (instance_ == nullptr)
        {
            std::lock_guard<std::mutex> lock(classMutex_);
            if (instance_ == nullptr)
            {
                instance_ = new ConnectionPool(host, username, password, database,
                                               maxConnections, minConnections, maxIdleTime);
            }
        }
        return *instance_;
    }
    // 禁止拷贝构造和赋值运算符
    ConnectionPool(const ConnectionPool &) = delete;
    ConnectionPool &operator=(const ConnectionPool &) = delete;

    /**
     * @brief 获取数据库连接
     * @return 数据库连接对象指针
     */
    sql::Connection *getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // 连接池为空，且已经到达最大连接数时，进行同步
        while (connections_.empty() && currConnections_ == maxConnections_)
        {
            cv_.wait(lock);
        }
        sql::Connection *connection = nullptr;
        // 连接池非空，从连接池获取
        if (!connections_.empty())
        {
            connection = connections_.front();
            connections_.pop_front();
        }
        else if (currConnections_ < maxConnections_) // 未达到最大连接数，新建sql连接
        {
            connection = getNewConnection();
        }

        return connection;
    }

    /**
     * @brief 释放数据库连接，将连接返回连接池
     * @param connection 数据库连接对象指针
     */
    void releaseConnection(sql::Connection *connection)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        connections_.push_back(connection);
        cv_.notify_one();
    }

private:
    /**
     * @brief 构造函数，初始化连接池
     * @param host 数据库主机名
     * @param username 数据库用户名
     * @param password 数据库密码
     * @param database 数据库名称
     * @param maxConnections 最大连接数
     * @param maxIdleTime 连接空闲超时时间（秒）
     */
    ConnectionPool(const std::string &host = "127.0.0.1", const std::string &username = "oj_client",
                   const std::string &password = "btjz.88ka.cn", const std::string &database = "oj",
                   int maxConnections = 10, int minConnections = 5, int maxIdleTime = 5)
        : host_(host), username_(username), password_(password), database_(database),
          maxConnections_(maxConnections), minConnections_(minConnections), currConnections_(0),
          maxIdleTime_(maxIdleTime), running_(true)
    {
        initializeConnections();  // 启动sql连接
        startConnectionMonitor(); // 启动监管线程
    }

    /**
     * @brief 析构函数，清理连接池资源
     */
    ~ConnectionPool()
    {
        stopConnectionMonitor();

        for (auto connection : connections_)
        {
            delete connection;
        }

        instance_ = nullptr;
        currConnections_ = 0;
    }

    /**
     * @brief 获取新的sql连接对象
     * 
     */
    sql::Connection *getNewConnection()
    {
        // // 创建数据库驱动和连接对象
        // sql::mysql::MySQL_Driver *driver;
        // sql::Connection *connection;

        // // 创建 MySQL 驱动程序实例
        // driver = sql::mysql::get_mysql_driver_instance(); // 全局静态的，不需要释放

        // 创建 MySQL 驱动程序实例
        static sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance(); // 获取到的是全局静态的，不需要释放
        // 创建数据库连接对象
        sql::Connection *connection;

        // 登录和连接数据库
        connection = driver->connect(host_, username_, password_);
        connection->setSchema(database_);
        ++currConnections_;

        return connection;
    }

    /**
     * @brief 初始化连接池，创建指定数量的数据库连接
     */
    void initializeConnections()
    {
        for (int i = 0; i < minConnections_; ++i)
        {
            auto connection = getNewConnection();
            // 将连接添加到连接池
            connections_.push_back(connection);
            lastUsedTimes_[connection] = std::chrono::system_clock::now();
        }
    }

    /**
     * @brief 启动连接监控线程，定期检查并回收超时的连接
     */
    void startConnectionMonitor()
    {
        monitorThread_ = std::thread([this]()
                                     {
                                         while (running_)
                                         {
                                             // 线程暂停最大空闲时间
                                             std::this_thread::sleep_for(std::chrono::seconds(maxIdleTime_));
                                             // 连接池容量不得小于minConnections_
                                             if (connections_.size() > minConnections_)
                                             {
                                                 // 连接池加锁
                                                 std::unique_lock<std::mutex> lock(mutex_);
                                                 // 获取当前时间
                                                 auto currentTime = std::chrono::system_clock::now();
                                                 // 遍历连接池
                                                 for (auto iter = connections_.begin(); iter != connections_.end() && connections_.size() > minConnections_;)
                                                 {
                                                     // 判断当前连接的空闲时间
                                                     auto connection = *iter;
                                                     auto lastUsedTime = lastUsedTimes_[connection];
                                                     auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUsedTime).count();
                                                     // 超时
                                                     if (idleTime >= maxIdleTime_)
                                                     {
                                                         // 超时的连接需要被回收
                                                         iter = connections_.erase(iter); // erase返回后一个元素的迭代器给iter
                                                         --currConnections_;
                                                         delete connection;
                                                         lastUsedTimes_.erase(connection);
                                                     }
                                                     else
                                                     {
                                                         ++iter;
                                                     }
                                                 }
                                             }
                                         }
                                     });
    }

    /**
     * @brief 停止连接监控线程
     */
    void stopConnectionMonitor()
    {
        running_ = false;
        monitorThread_.join();
    }

private:
    std::string host_;                                                                           // 数据库主机名
    std::string username_;                                                                       // 数据库用户名
    std::string password_;                                                                       // 数据库密码
    std::string database_;                                                                       // 数据库名称
    int currConnections_;                                                                        // 当前连接数
    int maxConnections_;                                                                         // 最大连接数
    int minConnections_;                                                                         // 最小连接数
    int maxIdleTime_;                                                                            // 连接空闲超时时间（秒）
    std::deque<sql::Connection *> connections_;                                                  // 连接池中的连接队列
    std::unordered_map<sql::Connection *, std::chrono::system_clock::time_point> lastUsedTimes_; // 连接的最后使用时间
    std::mutex mutex_;                                                                           // 互斥锁，保证线程安全
    std::condition_variable cv_;                                                                 // 条件变量，用于连接获取等待，进行同步
    std::thread monitorThread_;                                                                  // 连接监控线程
    bool running_;                                                                               // 连接监控线程运行标志

    // inline 避免重复定义
    static inline std::mutex classMutex_;                                              // 单例模式互斥锁，类内生命
    static inline ConnectionPool *instance_ = nullptr;                                                     // 类的唯一实例
};
/*
* static inline 定义静态成员变量时，不再需要类外定义 
ConnectionPool* ConnectionPool::instance_ = nullptr;
std::mutex ConnectionPool::mutex_;
*/


// int main()
// {
//     // 连接数据库
//     sql::mysql::MySQL_Driver *driver;
//     sql::Connection *connection;

//     driver = sql::mysql::get_mysql_driver_instance();
//     connection = driver->connect("127.0.0.1:3306", "oj_client", "btjz.88ka.cn");  // 替换为实际的数据库主机、用户名和密码

//     // 选择数据库
//     connection->setSchema("oj");  // 替换为实际的数据库名称

//     // 执行查询语句
//     sql::PreparedStatement *preparedStatement;
//     sql::ResultSet *resultSet;

//     preparedStatement = connection->prepareStatement("SELECT * FROM questions");
//     resultSet = preparedStatement->executeQuery();

//     // 遍历结果集并输出内容
//     while (resultSet->next())
//     {
//         // 根据表结构逐列获取数据
//         int id = resultSet->getInt("number");
//         std::string title = resultSet->getString("title");
//         std::string star = resultSet->getString("star");

//         // 输出查询结果
//         std::cout << "ID: " << id << ", Question: " << title << ", Answer: " << star << std::endl;
//     }

//     // 释放资源
//     delete resultSet;
//     delete preparedStatement;
//     delete connection;

//     return 0;
// }