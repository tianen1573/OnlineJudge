// #include <iostream>
// #include <mysql_driver.h>
// #include <mysql_connection.h>
// #include <unordered_map>
// #include <string>
// #include <ctime>
// #include "./comm/httplib.h"

// // MySQL 数据库连接参数
// const std::string dbHost = "localhost";
// const std::string dbUser = "root";
// const std::string dbPassword = "password";
// const std::string dbName = "mydatabase";

// // 用户信息结构体
// struct UserInfo {
//     std::string username;
//     std::string password;
// };

// // 会话信息结构体
// struct SessionInfo {
//     std::string sessionId;
//     std::string username;
//     time_t expiryTime;
// };

// // MySQL 数据库连接
// sql::mysql::MySQL_Driver* driver;
// std::unique_ptr<sql::Connection> connection;

// // 会话管理器类
// class SessionManager {
// private:
//     std::unordered_map<std::string, SessionInfo> sessions;
//     const int sessionExpiryTime = 60;  // 会话过期时间（单位：秒）

//     std::string generateSessionId() {
//         const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//         const int length = 16;
//         std::string sessionId;

//         srand(static_cast<unsigned int>(time(nullptr)));
//         for (int i = 0; i < length; ++i) {
//             sessionId += charset[rand() % charset.length()];
//         }

//         return sessionId;
//     }

// public:
//     std::string createSession(const std::string& username) {
//         std::string sessionId = generateSessionId();
//         time_t expiryTime = time(nullptr) + sessionExpiryTime;

//         SessionInfo sessionInfo;
//         sessionInfo.sessionId = sessionId;
//         sessionInfo.username = username;
//         sessionInfo.expiryTime = expiryTime;

//         sessions[sessionId] = sessionInfo;

//         return sessionId;
//     }

//     bool isSessionValid(const std::string& sessionId) {
//         if (sessions.count(sessionId) > 0) {
//             time_t currentTime = time(nullptr);
//             time_t expiryTime = sessions[sessionId].expiryTime;

//             if (currentTime < expiryTime) {
//                 sessions[sessionId].expiryTime = currentTime + sessionExpiryTime;
//                 return true;
//             } else {
//                 sessions.erase(sessionId);
//             }
//         }

//         return false;
//     }

//     std::string getUsername(const std::string& sessionId) {
//         if (sessions.count(sessionId) > 0) {
//             return sessions[sessionId].username;
//         }

//         return "";
//     }
// };

// // 用户数据库
// std::unordered_map<std::string, UserInfo> users;

// // 初始化数据库连接
// void initDatabase() {
//     driver = sql::mysql::get_mysql_driver_instance();
//     connection = std::unique_ptr<sql::Connection>(driver->connect(dbHost, dbUser, dbPassword));
//     connection->setSchema(dbName);
// }

// // 登录请求处理函数
// void handleLogin(const httplib::Request& req, httplib::Response& res) {
//     std::string username = req.get_param_value("username");
//     std::string password = req.get_param_value("password");

//     sql::PreparedStatement* statement = connection->prepareStatement("SELECT * FROM users WHERE username=? AND password=?");
//     statement->setString(1, username);
//     statement->setString(2, password);

//     sql::ResultSet* result = statement->executeQuery();
//     if (result->next()) {
//         std::string sessionId = sessionManager.createSession(username);
//         res.set_header("Set-Cookie", "session_id=" + sessionId);
//         res.set_content("Login successful", "text/plain");
//     } else {
//         res.set_content("Invalid username or password", "text/plain");
//     }

//     delete result;
//     delete statement;
// }

// // 注册请求处理函数
// void handleRegister(const httplib::Request& req, httplib::Response& res) {
//     std::string username = req.get_param_value("username");
//     std::string password = req.get_param_value("password");

//     sql::PreparedStatement* statement = connection->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)");
//     statement->setString(1, username);
//     statement->setString(2, password);

//     statement->execute();
//     res.set_content("Registration successful", "text/plain");

//     delete statement;
// }

// // 身份验证中间件
// void authenticate(const httplib::Request& req, httplib::Response& res, std::function<void()> next) {
//     std::string sessionId = req.get_header_value("Cookie");
//     if (!sessionId.empty() && sessionId.substr(0, 11) == "session_id=") {
//         sessionId = sessionId.substr(```cpp
// ... 11);
//         sessionId = sessionId.substr(11);

//         if (sessionManager.isSessionValid(sessionId)) {
//             std::string username = sessionManager.getUsername(sessionId);
//             req.set_header("X-Username", username);
//             next();
//             return;
//         }
//     }

//     res.set_content("Unauthorized", "text/plain");
// }

// int main() {
//     // 初始化数据库连接
//     initDatabase();

//     // 创建会话管理器
//     SessionManager sessionManager;

//     // 创建 HTTP 服务器
//     httplib::Server server;

//     // 处理登录请求
//     server.Post("/login", handleLogin);

//     // 处理注册请求
//     server.Post("/register", handleRegister);

//     // 添加身份验证中间件
//     server.set_base_dir("./public");
//     server.set_logger([](const httplib::Request& req, const httplib::Response& res) {
//         std::cout << req.method << " " << req.path << " -> " << res.status << std::endl;
//     });
//     server.use(authenticate);

//     // 启动服务器
//     server.listen("localhost", 8080);

//     return 0;
// }