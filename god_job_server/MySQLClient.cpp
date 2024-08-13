#include "MySQLClient.h"

MySQLClient::MySQLClient(const std::string& host, const std::string& user, const std::string& password, const std::string& database)
    : host(host), user(user), password(password), database(database), conn(mysql_init(NULL)), res(NULL) {
}

MySQLClient::~MySQLClient() {
    disconnect();
}

bool MySQLClient::connect() {
    if (conn == NULL) {
        std::cerr << "mysql_init() failed\n";
        return false;
    }

    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0) == NULL) {
        std::cerr << "mysql_real_connect() failed\n";
        std::cerr << mysql_error(conn) << std::endl;
        mysql_close(conn);
        conn = NULL;
        return false;
    }
    if (mysql_set_character_set(conn, "utf8mb4")) {
       
        return false;
    }

    return true;
}

void MySQLClient::disconnect() {
    if (conn) {
        mysql_close(conn);
        conn = NULL;
    }
    if (res) {
        mysql_free_result(res);
        res = NULL;
    }
}

bool MySQLClient::query(const std::string& query) {
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "QUERY ERROR: " << mysql_error(conn) << std::endl;
        return false;
    }

    res = mysql_store_result(conn);
    if (res == NULL) {
        std::cerr << "STORE RESULT ERROR: " << mysql_error(conn) << std::endl;
        return false;
    }

    return true;
}

void MySQLClient::printResults() const {
    if (res == NULL) {
        std::cerr << "No results to print\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    // Print the column names
    for (int i = 0; i < num_fields; ++i) {
        std::cout << fields[i].name << "\t";
    }
    std::cout << std::endl;

    // Print the rows
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << row[i] << "\t";
        }
        std::cout << std::endl;
    }
}
