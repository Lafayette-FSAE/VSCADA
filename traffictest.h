#ifndef TRAFFICTEST_H
#define TRAFFICTEST_H
#include <QTimer>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include "datacontrol.h"
//#include "canbus_interface.h"
//#include "usb7402_interface.h"
//#include "gpio_interface.h"

using namespace std;

class TrafficTest : public QObject
{
    Q_OBJECT

public:
    TrafficTest(map<int, meta *> can, vector<meta*> gpio, vector<meta*> i2c, vector<meta *> usb, int cRate, int gRate, int uRate, DataControl * ctrl);
    ~TrafficTest();

    void startTests();

    DataControl * dataCtrl;

    QTimer * canTimer = new QTimer;
    QTimer * usbTimer = new QTimer;
    QTimer * gpioTimer = new QTimer;

    int canRate;
    int usbRate;
    int gpioRate;
    vector<meta*> gpioSensors;
    vector<meta*> usbSensors;
    map<int,meta *> canSensorMap;
    vector<meta*> i2cSensors;

public slots:
    void sendCANItems();
    void sendGPIOItems();
    void sendI2CItems();
    void sendUSBItems();
};

#endif // TRAFFICTEST_H
