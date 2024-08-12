#pragma once
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <json.h> // 确保使用 JsonCpp 的正确路径
#include <string>
#include <sstream> // 引入 istringstream
#include "quest_ack.h"
#include <iostream>
#include <cstring>
#include <boost/bimap.hpp>

#define singleModel Model::GetInstance()
using namespace muduo;
using namespace muduo::net;
class Model // 使用 PascalCase 作为类名
{
public:
    static Model* GetInstance() // 方法名使用 PascalCase
    {
        static Model uniqueModel;
        return &uniqueModel;
    }

    void handleQuest(const std::string& jsonString,const TcpConnectionPtr& conn)
    {
        Json::Value* root = JsonParse(jsonString);
        int type =(*root)["type"].asInt();
        switch (type)
        {
        case logInQuest:
            handleLogInQuest(root,conn);
            break;
        }

        delete root;

    }
    void handleLogInQuest(Json::Value* root,const TcpConnectionPtr& conn)
    {
        // 访问解析后的 JSON 数据
        std::string account = (*root)["account"].asString();
        std::string password = (*root)["password"].asString();
        std::string sql = "select password from users where account='" + account + "'";
        std::string sql1 = "select account2_2,name from buddy_buddy join users on buddy_buddy.account2_2 = users.account where account2_1 = '" + account + "'";

        MySQLClient db("localhost", "root", "xqdeqqmima0721", "godJobDb");
        if (!db.connect()) {
            return;
        }
        bool loginSucces = false;
        //构建json对象
        
        
        if (db.query(sql)) {
            // 获取第一行
            MYSQL_ROW row = mysql_fetch_row(db.res);
            if (row != nullptr) {
                // 获取第一列的值
                std::string firstColumnValue = row[0] ? row[0] : "NULL"; // 检查空指针
                if (firstColumnValue == password)
                {
                    loginSucces = true;
                    account_connMap.insert({ account,conn });
                }
            }
            else {
                std::cerr << "No rows found in result set.\n";
            }
        }
        Json::Value rootAck;
        rootAck["type"] = logInAck;
        rootAck["isSuccess"] = loginSucces;
        if (loginSucces)
        {
            Json::Value buddys(Json::arrayValue);
            if (db.query(sql1)) {
                unsigned int num_fields = mysql_num_fields(db.res);
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(db.res))) {
                  Json::Value buddy;
                  buddy["acount"] = row[0];//好友账号
                  buddy["name"] = row[1];//好友昵称
                  buddys.append(buddy);
                }
            }

            rootAck["buddys"] = buddys;
        }
      
        db.disconnect();

        sendAck(account_connMap.left.find(account)->second, rootAck);
     
    }
void sendAck(const TcpConnectionPtr& conn, Json::Value& root)
{
    // 将 JSON 对象序列化为字符串
    Json::StreamWriterBuilder writer;
    std::string sendbuf1 = Json::writeString(writer, root);

    //加入信息头长度
    unsigned int size = sendbuf1.size();
    //const char* message_len = (const char*)&size;
    const char* message_len = reinterpret_cast<const char*>(&size);

    std::string sendbuf(message_len, sizeof(size));
    sendbuf += sendbuf1;

    conn->send(sendbuf);
}
//调用词方法后记得析构
 Json::Value* JsonParse(const std::string& jsonString) // 使用 const 引用以避免复制
    {
        // 解析 JSON 数据
        Json::Value* root = new Json::Value();
        Json::CharReaderBuilder reader;
        std::string errs;

        std::istringstream ss(jsonString);
        if (Json::parseFromStream(reader, ss, root, &errs)) {
            return root;
        }
        else {
            delete root; // 避免内存泄漏
            return nullptr; // 解析失败时返回 nullptr
        }
    }
 void deleteConn(const TcpConnectionPtr& conn) // 使用 const 引用以避免复制
 {
     if (account_connMap.right.find(conn) != account_connMap.right.end())
     {
         account_connMap.right.erase(conn);
     }
 }

 private:
     boost::bimaps::bimap<std::string, TcpConnectionPtr> account_connMap;
    

};
