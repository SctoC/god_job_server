#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <cstring>
#include <json.h>
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
                std::string name = root["name"].asString();
                int age = root["age"].asInt();
                bool isStudent = root["is_student"].asBool();
                const Json::Value courses = root["courses"];

                std::cout << "Name: " << name << std::endl;
                std::cout << "Age: " << age << std::endl;
                std::cout << "Is Student: " << (isStudent ? "Yes" : "No") << std::endl;

                std::cout << "Courses: ";
                for (const auto& course : courses) {
                    std::cout << course.asString() << " ";
                }
                std::cout << std::endl;
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
