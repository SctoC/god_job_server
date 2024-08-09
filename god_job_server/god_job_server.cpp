#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <cstring>
#include <json.h>
#include "MySQLClient.h"
using namespace muduo;
using namespace muduo::net;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    }
    else
    {
        LOG_INFO << "Connection closed by " << conn->peerAddress().toIpPort();
    }
}
void send_loginSucces(const TcpConnectionPtr& conn,bool isSuccess)
{

    //构建json对象
    Json::Value root;
    root["type"] = 1;
    root["isSuccess"] = isSuccess;

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
void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    static bool toBeDataLen = true;//将要到达的是数据长度还是数据内容
    static unsigned int bytesToRecive = 4;  //将要到达的字节数
    while(buf->readableBytes() >= bytesToRecive)
    {
        if (toBeDataLen)//接收数据长度
        {
            std::memcpy(&bytesToRecive, buf->peek(), 4);
            buf->retrieve(4);
            toBeDataLen = false;//接下来接收数据
        }
        else//接收数据
        {
            std::string jsonData = buf->retrieveAsString(bytesToRecive);
            toBeDataLen = true;
            bytesToRecive = 4;

            // 解析 JSON 数据
            Json::Value root;
            Json::CharReaderBuilder reader;
            std::string errs;

            std::istringstream ss(jsonData);
            if (Json::parseFromStream(reader, ss, &root, &errs)) {
                // 访问解析后的 JSON 数据
                std::string account = root["account"].asString();
                std::string password = root["password"].asString();
                std::string sql = "select password from users where account='"+account+"'";
                
                MySQLClient db("localhost", "root", "xqdeqqmima0721", "godJobDb");
                if (!db.connect()) {
                    return ;
                }
                bool loginSucces = false;
                if (db.query(sql)) {
                    // 获取第一行
                    MYSQL_ROW row= mysql_fetch_row(db.res);
                    if (row != nullptr) {
                        // 获取第一列的值
                        std::string firstColumnValue = row[0] ? row[0] : "NULL"; // 检查空指针
                        if (firstColumnValue == password)
                            loginSucces = true;
                    }
                    else {
                        std::cerr << "No rows found in result set.\n";
                    }
                }
                db.disconnect();

            
               send_loginSucces(conn, loginSucces);
              
                //// 输出格式化的 JSON 数据
                //std::cout << "Received JSON: " << root.toStyledString() << std::endl;

            }
            else {
                std::cerr << "Failed to parse JSON: " << errs << std::endl;
            }
        }
    }
    //LOG_INFO << "Received " << buf->readableBytes() << " bytes from " << conn->peerAddress().toIpPort();
    //std::cout << buf << std::endl;
    /*conn->send(buf);*/

}

int main(int argc, char* argv[])
{
    // Set up logging
 /*   Logging::setLogLevel(Logger::INFO);*/

    std::cout << "hello,world" << std::endl;
    // Create EventLoop
    EventLoop loop;

    // Define server address
    InetAddress listenAddr(9527);
    TcpServer server(&loop, listenAddr, "EchoServer");

    // Set up callbacks
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    // Start server
    server.start();
    loop.loop();  // Enter the event loop

    return 0;
}
