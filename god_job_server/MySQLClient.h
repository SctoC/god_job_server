#pragma once
// moduo_example.h: ��׼ϵͳ�����ļ��İ����ļ�
// ����Ŀ�ض��İ����ļ���

#include <mysql/mysql.h>
#include <string>
#include <iostream>

class MySQLClient {
public:
    MySQLClient(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    ~MySQLClient();

    bool connect();
    void disconnect();
    bool query(const std::string& query);
    void printResults() const;

public:
    MYSQL* conn;
    MYSQL_RES* res;
    std::string host;
    std::string user;
    std::string password;
    std::string database;
};


