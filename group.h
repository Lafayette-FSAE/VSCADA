#ifndef SUBSYSTEMTHREAD_H
#define SUBSYSTEMTHREAD_H

#include <QThread>
#include <QObject>
#include <QLineEdit>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <QTimer>
#include <QQueue>
#include <QTime>
#include <iomanip>
#include <mutex>
#include "typedefs.h"
#include "math.h"
#include "dbtable.h"

class DataMonitor;

using namespace std;

class Group : public QObject
{
    Q_OBJECT
public:
    Group(vector<meta *> sensors, string id, bool charc);    //class object destructor
    virtual ~Group();                     //class object destructor

    DBTable *dbase;
    string get_curr_time();                         //get curent time
    void set_rate(int newRate);                     //sets rate at which data is checked
    void logMsg(string msg);                        //enqueue message for display
    string getProgramTime();
    vector<meta *> get_mainsensors();
    void setMonitor(DataMonitor * mtr);             //sets monitor object
    void setSystemTimer(QTime * timer);
    void checkError();

    bool isCharcterised;
    QTimer * timer;                                 //timer to implement sampling frequency
    QTime * systemTimer;
    int checkRate = 1000;                              //rate for checking for sensor updates
    string groupId;                             //identifies subsystem by name
    vector<meta *> mainSensorVector;
    bool error = false;

signals:                           //execute response to CAN
    void pushI2cData(uint32_t value);
    void pushGPIOData(int pin, int value);                            //execute response to GPIO
    void pushMessage(string msg);
    void initiateRxn(int rxnCode);                               //execute configured reaction
    void updateDisplay(meta * sensor);
    void sendCANData(int address, uint64_t data, int size);
    void updateEditColor(string color, meta *sensor);
    void updateHealth();

public slots:
    void createGroupTable();
    void charcGroup();

};

#endif // SUBSYSTEMTHREAD_H
