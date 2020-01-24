#include "group.h"

/**
 * @brief Group::SubsystemThread - class constructor
 * @param mtr - data monitor module
 * @param sensors - vector of subsystem sensors configured
 */
Group::Group(vector<meta *> sensors, string id, DBTable * dbtool, bool charc){
    error=false;
    groupId = id;
    mainSensorVector = sensors;
    dbase = dbtool;
    isCharcterised=charc;
    if(isCharcterised)
      createGroupTable();
}

/**
 * @brief Group::~SubsystemThread - class destructor
 */
Group::~Group(){

}

void Group::setSystemTimer(QTime *timer){
    systemTimer = timer;
}
/*
*returns time since program started in seconds
*/string Group::getProgramTime(){
    int timeElapsed = systemTimer->elapsed();
    double time = static_cast<double>(timeElapsed)/1000;
    ostringstream streamObj;
    streamObj << std::fixed;
    streamObj << std::setprecision(3);
    streamObj << time;
    return streamObj.str();
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
    for (uint i = 0; i < mainSensorVector.size(); i++){
        if (mainSensorVector.at(i)->state != 0) return;
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
    pushMessage(msg);
}

void Group::createGroupTable(){
  if(mainSensorVector.size()<1){
    isCharcterised=false;
    return;
}
  string rowString;
  for(int i=0; i<mainSensorVector.size();i++){
        rowString=mainSensorVector.at(i)->id+"char not null,";
  }
  rowString.erase(rowString.end());
  dbase->create_sec(groupId+"charc",rowString);
  timer = new QTimer;
  connect(timer,SIGNAL(timeout()),this, SLOT(charcGroup()));
  timer->start(checkrate);
}

void Group::charcGroup(){
  string rowString;
  string colString;
  for(int i=0; i<mainSensorVector.size();i++){
        colString="'"+mainSensorVector.at(i)->id"','"
        rowString="'" + to_string(mainSensorVector.at(i)->calVal) + currsensor->unit+"','";
  }
  rowString.erase(rowString.end()-1);
  colString.erase(colString.end()-1);
  dbase->add_row_sec(groupId+"charc",colString,rowString);
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
