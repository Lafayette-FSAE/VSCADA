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
/* Used when calculating sensors using Polynomials*/
typedef struct{
    int exponent;
    double coefficient;
}poly;

/* The base type for all sensors. has multiple variables to cover
*  different types of sensors
*/
typedef struct{
    uint32_t primAddress;
    uint auxAddress;// used to locate which sensor is data is for
    uint offset;// used to tell which bits values come from

    int gpioPin;
    int motor;//used to differ tsi and Controller pckt unneeded with standardization
    int checkRate;//sensor checkrate
    int state; // used when sensor exceeds max or min
    int i2cAddress;// i^2c controls
    std::vector<uint32_t> i2cConfigs;
    uint8_t i2cReadPointer;
    int i2cReadDelay;
    int i2cDataField;
    int usbChannel;
    double minimum;//min and max sensor values
    double maximum;
    int respnum;
    double calConst;
    int sensorIndex;
    int trigger;// value change in sensor that signifies it needs to be stored
    double trigval;
    int reciprocol;	// add rec flag
    uint8_t precision;// decimal point
    double calMultiplier;
    double val;// value sensor holds before calculating
    double calVal;// value sensor holds after calculating
    std::vector<poly> calPolynomial;
    std::vector<std::string> groups;
    std::string unit;
    std::string sensorName;

    void calData(){
        calVal = static_cast<double>(val)*calMultiplier + calConst;
    }

	// added method by tony used in tsi cal
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

/*
* pretty self explainitory would benifit from being changed
*/
typedef struct{
    int value;
    uint offset;
    bool active;
    uint auxAddress;
    uint primAddress;
    std::string name;
}system_state;


/*
* the system's statemachine
*/
typedef struct{
    uint offset;
    uint auxAddress;
    uint primAddress;
    std::string name;
    std::vector<system_state *> states;
}statemachine;

/*
* Used to send can messages written in the config file
*/
typedef struct{
    int address;
    uint64_t data;
    uint64_t data2;
    int bytes;
    int rate_ms;
}canItem;

/*
* how scada reacts to different situations
*/
struct response{
  int responseIndex;
  std::string state;
  std::vector<int> sensors;
  std::string msg;
  uint64_t data;
  int gpioValue;
  int address;
  int gpioPin;
  int offset;
};

#endif // TYPEDEFS_H
