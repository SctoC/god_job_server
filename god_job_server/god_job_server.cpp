#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <iostream>
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
    LOG_INFO << "Received " << buf->readableBytes() << " bytes from " << conn->peerAddress().toIpPort();
    conn->send(buf);
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
