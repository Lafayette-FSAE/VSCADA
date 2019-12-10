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

class DataMonitor;

using namespace std;

class Group : public QObject
{
    Q_OBJECT
public:
    Group(vector<meta *> sensors, string id, vector<response> respVector, vector<meta *> mainMeta);    //class object destructor
    virtual ~Group();                     //class object destructor

    void stop();                                    //stops data collection
    void start();                                   //starts data collection
    vector<int> get_data();                         //initializes GLV data vector
    string get_curr_time();                         //get curent time
    void set_rate(int newRate);                     //sets rate at which data is checked
    void logMsg(string msg);                        //enqueue message for display
    string getProgramTime();
    vector<meta *> get_mainsensors();
    vector<meta *> get_metadata();                  //retrieves a configured list of sensors
    void setMonitor(DataMonitor * mtr);             //sets monitor object
    void setSystemTimer(QTime * timer);
    void checkError();

    mutex procSensorMutex;
    QTimer * timer;                                 //timer to implement sampling frequency
    DataMonitor * monitor;                          //pointer to a datamonitor object

    QThread * thread;
    QTimer * checkTmr;                              //timer placeholder for checking update frequencies
    QLineEdit * lineEdit;                           //lineEdif placeholder for sensor-specific line edits
    QTime * systemTimer;

    int checkRate = 0;                              //rate for checking for sensor updates
    string groupId;                             //identifies subsystem by name

    vector<canItem> broadCastVector;

    vector<meta *> sensorMeta;                      //cooling sensor metadata
    vector<meta *> mainSensorVector;
    vector<QLineEdit *> edits;                      //lineEdits for displaying data
    vector<QLineEdit *> controlEdits;
    vector<QTimer *> editTimers;                    //stores checkTimers
    vector<response> responseVector;                //stores configured responses

    QQueue<meta *> sensorQueue;
    QQueue<string> * msgQueue;                      //queue to store messages for display
    bool error = false;
    QQueue<response> * respCANQueue;                //queue for CAN responses
    QQueue<response> * respGPIOQueue;               //queue for gpio responses

    bool running = true;                            //to control running of collection thread
    vector<int> rawData;                            //cooling sensor data

signals:                           //execute response to CAN
    void pushI2cData(uint32_t value);
    void pushGPIOData(int pin, int value);                            //execute response to GPIO
    void pushMessage(string msg);
    void initiateRxn(int rxnCode);                               //execute configured reaction
    void updateDisplay(meta * sensor);
    void sendCANData(int address, uint64_t data, int size);
    void updateEditColor(string color, meta *sensor);
    void updateHealth();
};

#endif // SUBSYSTEMTHREAD_H
