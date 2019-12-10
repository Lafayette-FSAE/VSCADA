#include "dbtable.h"

DBTable::DBTable(std::string name){
    db_name = name;
    db_location = "./";
    Open_database();
}
DBTable::DBTable(std::string location, std::string name){
    db_name = name;
    db_location =location;
    Open_database();

}
//close the database
DBTable::~DBTable(){
    sqlite3_close(database);
    database=nullptr;
}
string DBTable::section="";
//opens the database
void DBTable::Open_database(){
    std::string full_name = db_location + "/" + db_name;
    sqlite3_open(full_name.c_str(), &database);
}

int DBTable::create(string table,string column) {
    if(column==""||table=="")
        return -1;
    std::string sql_create;
    int   returnCode=0;
    char *ErrMsg;
    sql_create +=  "create table if not exists "+ table+ "( "+column;
    sql_create +=    ");";
    returnCode = sqlite3_exec(database,sql_create.c_str(),nullptr,this,&ErrMsg);
    return returnCode;
}

int DBTable::create_sec(string table,string column) {
    if(column==""||table=="")
        return -1;
    std::string sql_create;
    int   returnCode=0;
    char *ErrMsg;
    sql_create +=  "create table if not exists "+ table+ "( section char not null,"+column;
    sql_create +=    ");";
    returnCode = sqlite3_exec(database,sql_create.c_str(),nullptr,this,&ErrMsg);
    return returnCode;
}

bool DBTable::add_row(std::string table, std::string colums,std::string rows) {
    int   returnCode = 0;
    char *ErrMsg;
    std::string add_row;
    add_row  = "INSERT INTO " + table +"("+ colums +")"+ " VALUES ("+rows +");";
    //cout<<add_row<<endl;
    returnCode = sqlite3_exec(database,add_row.c_str(),nullptr,this,&ErrMsg);
    return returnCode;
}
bool DBTable::add_row_sec(std::string table, std::string colums,std::string rows) {
    int   returnCode = 0;
    char *ErrMsg;
    std::string add_row;
    int sec;
    if(section=="")
        sec=0;
    else{
        sec=stoi(section)+1;
    }
    add_row  = "INSERT INTO " + table +"(section,"+ colums +")"+ " VALUES ('"+to_string(sec)+"',"+rows +");";
    //cout<<add_row<<endl;
    returnCode = sqlite3_exec(database,add_row.c_str(),nullptr,this,&ErrMsg);
    return returnCode;
}

bool DBTable::select_all() {
    int   returnCode = 0;
    char *ErrMsg;
    std::string sql_select_all = "SELECT * FROM "+ db_table+ ";";
    returnCode = sqlite3_exec(database,sql_select_all.c_str(),nullptr,this,&ErrMsg);
    return returnCode;
}

int DBTable::SecNum(string table){
    int   returnCode = 0;
    char *ErrMsg;
    string section  = "SELECT * FROM "+ table+ ";";
    returnCode = sqlite3_exec(database,section.c_str(),SecNum_callback,this,&ErrMsg);
    return returnCode;
}

int DBTable::SecNum_callback(void *unused, int count, char **data, char **columns)
{
    int idx;
    string temp="section";
    cout << "There are "<< to_string(count) << "column(s)\n";
    for (idx = 0; idx < count; idx++) {
        if(temp.compare(columns[idx])==0){
            section=data[idx];
            cout <<section<<endl;
        }
        cout << "The data in column "<<columns[idx]<<" is:"<<data[idx]<< "\n";
    }
    printf("\n");
    return 0;
}
