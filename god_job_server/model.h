#pragma once
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include <json.h> // ȷ��ʹ�� JsonCpp ����ȷ·��
#include <string>
#include <sstream> // ���� istringstream
#include "quest_ack.h"
#include <iostream>
#include <cstring>
#include <boost/bimap.hpp>

#define singleModel Model::GetInstance()
using namespace muduo;
using namespace muduo::net;
class Model // ʹ�� PascalCase ��Ϊ����
{
public:
    static Model* GetInstance() // ������ʹ�� PascalCase
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
        case sendMessageQuest:
            handlesendMessageQuest(root);
            break;
        case sendGroupMessageQuest:
            handlesendGroupMessageQuest(root);
            break;
            
        }

        delete root;

    }
    void handleLogInQuest(Json::Value* root,const TcpConnectionPtr& conn)
    {
        // ���ʽ������ JSON ����
        std::string account = (*root)["account"].asString();
        std::string password = (*root)["password"].asString();
        std::string sql = "select password from users where account='" + account + "'";
        std::string sql1 = "select account2_2,name from buddy_buddy join users on buddy_buddy.account2_2 = users.account where account2_1 = '" + account + "'";
        std::string sql2 = "select groupMember.groupId, groupName from groupMember join groupIfo on groupMember.groupId = groupIfo.groupId   where userAcount = '"+ account + "'";
        std::string sql3 = "select groupMember.userAcount, users.name from groupMember join users on groupMember.userAcount = users.account where groupId = '";

        MySQLClient db("localhost", "root", "xqdeqqmima0721", "godJobDb");
        if (!db.connect()) {
            return;
        }
        bool loginSucces = false;
        //����json����
        
        
        if (db.query(sql)) {
            // ��ȡ��һ��
            MYSQL_ROW row = mysql_fetch_row(db.res);
            if (row != nullptr) {
                // ��ȡ��һ�е�ֵ
                std::string firstColumnValue = row[0] ? row[0] : "NULL"; // ����ָ��
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
        MYSQL_RES* res = nullptr;
        if (loginSucces)
        {
            Json::Value buddys(Json::arrayValue);
            if (db.query(sql1)) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(db.res))) {
                  Json::Value buddy;
                  buddy["account"] = row[0];//�����˺�
                  buddy["name"] = row[1];//�����ǳ�
                  buddys.append(buddy);
                }
            }
            db.freeRes();
            rootAck["buddys"] = buddys;

            Json::Value groups(Json::arrayValue);
            if (db.query(sql2)) {
                MYSQL_ROW row;
                res = db.res;
                while ((row = mysql_fetch_row(res))) {
                    Json::Value group;
                    group["groupId"] = row[0];//Ⱥ��ID
                    group["groupName"] = row[1];//Ⱥ���ǳ�

                    Json::Value buddys(Json::arrayValue);
                    std::string sql4 = sql3 + row[0] + "'";
               
                    if (db.query(sql4)) {
                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(db.res))) {
                            Json::Value buddy;
                            buddy["account"] = row[0];//Ⱥ��Ա�˺�
                            buddy["Name"] = row[1];//Ⱥ��Ա�ǳ�

                            buddys.append(buddy);
                        }
                    }
                    db.freeRes();
                    group["members"] = buddys;//Ⱥ��Ա
                    groups.append(group);
                }
            }
            rootAck["groups"] = groups;
        }
        
        if (res) {
            mysql_free_result(res);
            res = NULL;
        }
        db.disconnect();

        sendAck(account_connMap.left.find(account)->second, rootAck);
     
    }
    void handlesendMessageQuest(Json::Value* root)
    {
        // ���ʽ������ JSON ����
      
        std::string receive_account = (*root)["receive_account"].asString();

        if (account_connMap.left.find(receive_account) != account_connMap.left.end())
        {
 
            sendAck(account_connMap.left.find(receive_account)->second, *root);
        }
        //����洢������Ϣ   
    }
    void handlesendGroupMessageQuest(Json::Value* root)
    {
        // ���ʽ������ JSON ����

        std::string send_account = (*root)["send_account"].asString();
        std::string group_id = (*root)["group_id"].asString();

        //����Ⱥ���г�Ա
        std::string sql = "select userAcount from groupMember  where groupId='" + group_id + "'";
        MySQLClient db("localhost", "root", "xqdeqqmima0721", "godJobDb");
        if (!db.connect()) {
            return;
        }
        std::vector<std::string> memberAccounts;

        if (db.query(sql)) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(db.res))) {
                memberAccounts.push_back(row[0]);
            }
        }
        db.disconnect();

        for (int i = 0; i < memberAccounts.size(); i++)
        {
            if (memberAccounts[i] != send_account)
            {
                if (account_connMap.left.find(memberAccounts[i]) != account_connMap.left.end())
                {
                    sendAck(account_connMap.left.find(memberAccounts[i])->second, *root);
                }
            }
        }

        //����洢������Ϣ   
    }
void sendAck(const TcpConnectionPtr& conn, Json::Value& root)
{
    // �� JSON �������л�Ϊ�ַ���
    Json::StreamWriterBuilder writer;
    std::string sendbuf1 = Json::writeString(writer, root);

    //������Ϣͷ����
    unsigned int size = sendbuf1.size();
    //const char* message_len = (const char*)&size;
    const char* message_len = reinterpret_cast<const char*>(&size);

    std::string sendbuf(message_len, sizeof(size));
    sendbuf += sendbuf1;

    conn->send(sendbuf);
}
//���ôʷ�����ǵ�����
 Json::Value* JsonParse(const std::string& jsonString) // ʹ�� const �����Ա��⸴��
    {
        // ���� JSON ����
        Json::Value* root = new Json::Value();
        Json::CharReaderBuilder reader;
        std::string errs;

        std::istringstream ss(jsonString);
        if (Json::parseFromStream(reader, ss, root, &errs)) {
            return root;
        }
        else {
            delete root; // �����ڴ�й©
            return nullptr; // ����ʧ��ʱ���� nullptr
        }
    }
 void deleteConn(const TcpConnectionPtr& conn) // ʹ�� const �����Ա��⸴��
 {
     if (account_connMap.right.find(conn) != account_connMap.right.end())
     {
         account_connMap.right.erase(conn);
     }
 }

 private:
     boost::bimaps::bimap<std::string, TcpConnectionPtr> account_connMap;
    

};
