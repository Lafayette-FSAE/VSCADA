#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#define RUN 0
#define TEST 1

#define DASH_DISP 0
#define BACK_DISP 1

#define SUM 0
#define DIFF 1
#define MUL 2
#define DIV 3
#define AVG 4
#define MAX 5
#define MIN 6

#define WATCHDOG_PERIOD 10

#define DB_BUF_SIZE 100
#define CAN_FRAME_LIMIT 20

typedef struct{
    int exponent;
    double coefficient;
}poly;

typedef struct{
    int motor;
    double val;
    uint offset;
    int gpioPin;
    int checkRate;
    double calVal;
    int maxRxnCode;
    int minRxnCode;
    uint auxAddress;
    int i2cAddress;
    std::vector<uint32_t> i2cConfigs;
    uint8_t i2cReadPointer;
    int i2cReadDelay;
    int i2cDataField;
    int usbChannel;
    double minimum;
    double maximum;
    double calConst;
    int sensorIndex;
    int normRxnCode;
    int trigger;
    double trigval;
    int reciprocol;	// add rec flag
    uint32_t primAddress;
    uint8_t precision;
    double calMultiplier;
    int state;
    std::vector<poly> calPolynomial;
    std::vector<std::string> groups;
    std::string unit;
    std::string sensorName;
    void updateVal(int newVal)
    {
        val = newVal;
    }

    void calData(){
        calVal = static_cast<double>(val)*calMultiplier + calConst;
    }
	
	// add method
	void calRec(){
		if (reciprocol == 1) {
			if (calVal != 0) {
				calVal = 1.0 / calVal;
			}
			if (val != 0) {
				val = 1.0 / val;
			}
		}
    }
}meta;

typedef struct{
    uint offset;
    int gpiopin;
    bool slider;
    bool button;
    int pressVal;
    int maxslider;
    int minslider;
    int releaseVal;
    int usbChannel;
    uint auxAddress;
    bool textField;
    uint primAddress;
    uint64_t sentVal;
    std::string name;
    std::string type;
    double multiplier;
}controlSpec;

typedef struct{
    int defVal;
    uint offset;
    int gpioPin;
    int gpioPair;
    int canValue;
    int gpioValue;
    uint auxAddress;
    std::string msg;
    uint primAddress;
    int responseIndex;
    int displayTarget;
}response;

typedef struct{
    int value;
    uint offset;
    bool active;
    uint auxAddress;
    uint primAddress;
    std::string name;
}system_state;

//typedef struct{
//    std::string name;
//    uint auxAddress;
//    uint offset;
//    int value;
//}condition;

typedef struct{
    uint offset;
    uint auxAddress;
    uint primAddress;
    std::string name;
    std::vector<system_state *> states;
    //std::vector<condition *> conditions;
}statemachine;

typedef struct{
    int address;
    uint64_t data;
    uint64_t data2;
    int bytes;
    int rate_ms;
}canItem;

typedef struct{
    int pin;
    int value;
    int mode;
    int rate_ms;
}gpioItem;

typedef struct{
    int address;
    int data;
    int rate_ms;
}i2cItem;

//typedef struct{
//    std::vector<canItem> bootCanCmds;
//    std::vector<uint32_t> bootI2cCmds;
//    std::vector<gpioItem> bootGPIOCmds;
//} bootloader;

//typedef struct{
//    int id;
//    int period;
//    std::vector<int> sensorIds;
//    int triggerSensor;
//    std::string triggerFSM;
//    std::string triggerState;
//    int startVal;
//    int stopVal;
//    std::string savePath;
//    std::string prefix;
//    bool active;
//    void * timer;
//} recordwindow;

#endif // TYPEDEFS_H
