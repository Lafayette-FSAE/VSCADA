#ifndef GPIO_INTERFACE_H
#define GPIO_INTERFACE_H
#include <QObject>
#include <QTimer>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "typedefs.h"
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

using namespace std;

class gpio_interface : public QObject
{
    Q_OBJECT

public:
    gpio_interface(vector<meta *> gpioSen, vector<meta *> i2cSen, vector<response> responses);
    ~gpio_interface();

    void stopGPIOCheck();
    void startGPIOCheck();
    int GPIORead(meta *sensor);
    int GPIOExport(int pin);
    int i2cInit(int address);
    int i2cRead(meta * sensor);
    int GPIOUnexport(int pin);
    void setSamplingRate(int newRate);
    int GPIODirection(int pin, int dir);

    int samplingRate = 1000;

    QTimer * timer;
    vector<meta *> i2cSensors;
    vector<meta *> gpioSensors;
    vector<int> i2cSlaveAddresses;
    vector<int> i2cFileDescriptors;
    vector<response> responseVector;

public slots:
    void gpioCheckTasks();
    void GPIOWrite(int pin, int value);
    int i2cWrite(int address, int data);

signals:
    void sensorValueChanged(meta * sensor);
};

#endif // GPIO_INTERFACE_H
