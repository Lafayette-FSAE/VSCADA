#ifndef DATACONTROL_H
#define DATACONTROL_H
#include <sqlite3.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <stack>
#include <bitset>
#include <iomanip>
#include <map>
#include "typedefs.h"
#include "group.h"
#include "usb7402_interface.h"
#include "gpio_interface.h"
#include "canbus_interface.h"
#include "dbtable.h"

#define OFF 0
#define IDLE_MODE 1
#define DRIVE_MODE 2

using namespace std;

class DataControl : public QObject
{
    Q_OBJECT
public:
    // Member function declarations
    DataControl(gpio_interface *gpio, canbus_interface *can, usb7402_interface * usb,
                map<string, Group *> subMap, vector<system_state *> stts, vector<statemachine *> FSMs,
                int mode, map<int,response> rspMap,
                map<uint32_t, vector<meta *> *> canMap,vector<canItem> cSyncs, map<int, meta *> sensMap);
    ~DataControl();

    void setMode(int md);
    string get_curr_time();
    string get_curr_date();
    void startSystemTimer();
    string getProgramTime();
    void saveSession(string name);
    void init_meta_vector(vector<meta> vctr);
    int change_system_state(system_state * newState);
    uint64_t LSBto64Spec(uint auxAddress, uint offset, uint64_t data);
    uint32_t isolateData64(uint auxAddress, uint offset, uint64_t data);
    void receiveData(meta * currSensor);
    void calibrateData(meta * currSensor);
    void checkThresholds(meta * sensor);
    void incrementSessionNumber();
    string removeSpaces(string &str);
    void save_all_data();
    void logData(meta * currSensor);

    // active submodule pointers
    DataMonitor * monitor;
    usb7402_interface * usb7204;
    vector<statemachine *> FSMs;
    map<int,meta *> sensorMap;
    QTime * systemTimer;
    vector<system_state *> states;
    gpio_interface * gpioInterface;
    canbus_interface * canInterface;
    map<int,response> responseMap;
    map<int,string> recordColStrings;
    map<string, Group *> subsystemMap;
    map<uint32_t,vector<meta *> *> canSensorGroup;
    vector<canItem> canSyncs;
    DBTable *dbase;
    QTimer * syncTimer;
    QTimer * watchdogTimer;
    vector<QTimer *> canSyncTimers;
    map<int, QTimer *> i2cSyncTimers;
    map<int, QTimer *> gpioSyncTimers;

    QTimer * recordTimer;

    // overall system status info
    int systemMode;
    int sessionNumber = 0;
    string modeName;
    string currState;
//    bootloader bootCmds;

public slots:
    void receive_sensor_data(meta * sensor);
    void executeRxn(int responseIndex);
    void deactivateLog(system_state * prevstate);
    void receive_can_data(uint32_t addr, uint64_t arr);
    void canSyncSlot();
    void feedWatchdog();
signals:
    void sendI2CData(int address, int data);
    void pushI2cData(uint32_t value);
    void pushMessage(string msg);
    void pushGPIOData(int pin, int value);
    void updateFSM(statemachine * currFSM);
    void activateState(system_state * newState);
    void sendCANData(int address, uint64_t data);
    void sendCANDataByte(int address, uint64_t data, int size);
    void deactivateState(system_state * prevstate);
    void sendToUSB7204(uint8_t channel, float voltage, bool*);
    void updateEdits(meta *);
    void updateDisplay(meta * sensor);
    void updateEditColor(string color, meta *sensor);
};
#endif // DATACONTROL_H
