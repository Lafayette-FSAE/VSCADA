#ifndef CONFIG_H
#define CONFIG_H
#include <sqlite3.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <map>
#include "typedefs.h"
#include "datacontrol.h"
#include "gpio_interface.h"
#include "canbus_interface.h"
#include "group.h"
#include <QtXml/QtXml>
#include "usb7402_interface.h"
#include "traffictest.h"


#define CONFIG_PRINT

using namespace std;

class Config
{
public:

    // member function declarations
    Config();
    ~Config();
    string get_curr_time();
    bool read_config_file_data();
    bool isInteger(const string & s);
    string removeSpaces(string &str);

    //system mode : CAR or DYNO
    int systemMode;
    int gpioRate;
    int usb7204Rate;
    int canRate;

    //dummy variables
    meta * storedSensor;
    statemachine * thisFSM;
    system_state * thisState;
    //condition * thisCondition;
    Group * grp;

    //dbase args strings
    string sensorColString;
    string sensorRowString;
    string systemColString;
    string systemRowString;

    //data vectors
    vector<meta *> canSensors;
    vector<meta *> usbSensors;
    vector<meta *> i2cSensors;
    vector<meta *> gpioSensors;
    vector<string> configErrors;
    vector<meta *> mainSensors;
    vector<canItem> canSyncs;

    map<int,meta *> canSensorMap;
    map<int,meta *> sensorMap;
    map<uint32_t, int> canAddressMap;
    vector<meta*> * canVectorItem;
    map<string, Group *> groupMap;
    map<uint32_t,vector<meta *> *> canSensorGroup;
    map<int,response> responseMap;
//    map<int,recordwindow *> recordSensorConfigs;
//    map<int,recordwindow *> recordStateConfigs;

    vector<statemachine *> FSMs;
    vector<system_state *> sysStates;

    //test module
    TrafficTest * trafficTest;

    // submodule declarations
    DataControl * dataCtrl;
    usb7402_interface * usb7204;
    gpio_interface * gpioInterface;
    canbus_interface * canInterface;
    vector<Group *> subsystems;
};
#endif // CONFIG_H
