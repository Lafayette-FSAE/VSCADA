#ifndef DBTABLE_H
#define DBTABLE_H

#include <iostream>
#include <string>
#include <sqlite3.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <set>
#include <map>

using namespace std;

class DBTable {
    std::string db_name;
    std::string db_location;
    std::string db_table;
    sqlite3 *database;
    bool t_exists;
public:

    DBTable();
    DBTable(std::string name);
    DBTable(std::string location, std::string name);
    ~DBTable();
    void Open_database();
    bool select_all();
    int create(string table, string column);
    int create_sec(string table, string column);
    bool add_row(std::string table, std::string colums,std::string rows);
    bool add_row_sec(std::string table, std::string colums,std::string rows);
    int SecNum(string table);
    static int SecNum_callback(void *unused, int count, char **data, char **columns);
    static string section;

};
#endif // DBTABLE_H
