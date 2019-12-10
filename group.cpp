#include "group.h"

/**
 * @brief Group::SubsystemThread - class constructor
 * @param mtr - data monitor module
 * @param sensors - vector of subsystem sensors configured
 */
Group::Group(vector<meta *> sensors, string id, vector<response> respVector, vector<meta *> mainMeta){
    msgQueue = new QQueue<string>;
    error=false;
    sensorMeta = sensors;
    groupId = id;
    responseVector = respVector;
    mainSensorVector = mainMeta;
}

/**
 * @brief Group::~SubsystemThread - class destructor
 */
Group::~Group(){

}

void Group::setSystemTimer(QTime *timer){
    systemTimer = timer;
}

string Group::getProgramTime(){
    int timeElapsed = systemTimer->elapsed();
    double time = static_cast<double>(timeElapsed)/1000;
    ostringstream streamObj;
    streamObj << std::fixed;
    streamObj << std::setprecision(3);
    streamObj << time;
    return streamObj.str();
}

/**
 * @brief Group::get_metadata -
 * @return
 */
vector<meta *> Group::get_metadata(){
    return sensorMeta;
}

/**
 * @brief Group::set_rate - changes the current rate at which data is checked
 * @param newRate
 */
void Group::set_rate(int newRate){
    checkRate = newRate;
}

/**
 * @brief Group::setMonitor - sets monitor object for class
 * @param mtr
 */
void Group::setMonitor(DataMonitor * mtr){
    monitor = mtr;
}

/**
 * @brief Group::get_mainMeta : retrieves main subsystem sensors
 * @return
 */
vector<meta *> Group::get_mainsensors(){
    return mainSensorVector;
}

void Group::checkError(){
    for (uint i = 0; i < sensorMeta.size(); i++){
        if (sensorMeta.at(i)->state != 0) return;
    }
    error = false;
}

/**
 * @brief Group::enqueueMsg - logs message in the database
 * @param msg
 */
void Group::logMsg(string msg){
    string colString = "time,reactionId,message";
    string rowString = "'" + getProgramTime() + "','console','" + msg + "'";
    //dbase->insert_row("system_log",colString,rowString);
    pushMessage(msg);
}

/**
 * @brief Group::get_data - retrieves subsystem raw data
 */
vector<int> Group::get_data(){
    return rawData;
}

/**
 * @brief Group::get_curr_time - retrieves current operation system time
 * @return
 */
string Group::get_curr_time(){
    time_t t = time(nullptr);
    struct tm now = *localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf),"%D_%T",&now);
    return buf;
}
