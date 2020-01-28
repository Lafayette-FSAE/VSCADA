#include "datacontrol.h"

/**
 * @brief DataControl::DataControl class control
 * @param gpio : GPIO interface module
 * @param can : CAN interface module
 * @param usb : USB7204 interface module
 * @param db : SQLITE database engine
 * @param threads : subsystem threads
 * @param stts : system statuses
 * @param FSM : system finite state machines
 * @param mode : system mode
 * @param ctrlSpecs : control specifications
 * @param sensors : system sensors
 * @param rsp : system responses
 */
DataControl::DataControl(gpio_interface * gpio, canbus_interface * can, usb7402_interface * usb,
                         map<string, Group *> subMap, vector<system_state *> stts, vector<statemachine *> FSM,
                         int mode, map<int, response> rspMap,
                         map<uint32_t, vector<meta *>*> canMap,vector<canItem> cSyncs,
                         map<int, meta*> sensMap){

    // set mode parameters
    systemMode = mode;
    if(mode == 1){
        modeName = "DYNO";
    } else if (mode == 0){
        modeName = "CAR";
    }

    // assign global objects
    FSMs = FSM;
    usb7204 = usb;
    states = stts;
    canInterface = can;
    responseMap = rspMap;
    subsystemMap = subMap;
    gpioInterface = gpio;
    systemTimer = new QTime;
    canSensorGroup = canMap;
    sensorMap = sensMap;
    dbase = new DBTable("SCADA.db");
    dbase->create_sec("sensor_data","id char not null,time char not null,"
                                    "sensor_value char not null");
    vector<string> cols;
    cols.push_back("recordindex");
    sessionNumber = 10;
    cout << "Record Index: " << sessionNumber << endl;
    startSystemTimer();
    //*****************************************//
    //********signal-slot connections*********//
    //***************************************//
    map<string, Group *>::iterator it;
    canSyncs= cSyncs;
    for (uint i = 0; i < canSyncs.size(); i++){
        syncTimer = new QTimer;
        connect(syncTimer, SIGNAL(timeout()), this, SLOT(canSyncSlot()));
        canSyncTimers.push_back(syncTimer);
        syncTimer->start(canSyncs.at(i).rate_ms);
    }

    for ( it = subsystemMap.begin(); it != subsystemMap.end(); it++ ){
        //connect(it->second, SIGNAL(initiateRxn(int)), this,SLOT(executeRxn(int)));
        it->second->setSystemTimer(systemTimer);
    }

    watchdogTimer = new QTimer;
    connect(watchdogTimer, SIGNAL(timeout()), this, SLOT(feedWatchdog()));
    watchdogTimer->start((WATCHDOG_PERIOD/2)*1000);

    connect(this, SIGNAL(pushGPIOData(int,int)), gpioInterface,SLOT(GPIOWrite(int,int)));
    connect(this, SIGNAL(sendI2CData(int,int)), gpioInterface,SLOT(i2cWrite(int,int)));
    connect(this, SIGNAL(deactivateState(system_state *)), this,SLOT(deactivateLog(system_state *)));
    connect(this, SIGNAL(sendToUSB7204(uint8_t, float, bool*)), usb7204, SLOT(writeUSBData(uint8_t, float, bool*)));
    connect(this, SIGNAL(sendCANData(int, uint64_t)), canInterface, SLOT(sendData(int, uint64_t)));
    connect(this, SIGNAL(sendCANDataByte(int, uint64_t,int)), canInterface, SLOT(sendDataByte(int, uint64_t,int)));
    connect(canInterface, SIGNAL(process_can_data(uint32_t,uint64_t)), this, SLOT(receive_can_data(uint32_t,uint64_t)));
    connect(usb7204, SIGNAL(sensorValueChanged(meta*)), this, SLOT(receive_sensor_data(meta*)));
    connect(gpioInterface, SIGNAL(sensorValueChanged(meta*)), this, SLOT(receive_sensor_data(meta*)));
    cout << "DATA CONTROL CONFIGURED" << endl;
}

DataControl::~DataControl(){

}

/**
 * @brief DataControl::startSystemTimer : starts internal clock
 */
void DataControl::startSystemTimer(){
    systemTimer->start();
}

/*
* Get time since programe started in seconds
*/
string DataControl::getProgramTime(){
    int timeElapsed = systemTimer->elapsed();
    double time = static_cast<double>(timeElapsed)/1000;
    ostringstream streamObj;
    streamObj << std::fixed;
    streamObj << std::setprecision(3);
    streamObj << time;
    return streamObj.str();
}

/*
* self explainatory
*/
void DataControl::feedWatchdog(){
    cout << "Feeding watchDog..." << endl;
    system("echo 0 > watchdog.txt");
}

/*
* Sends can message when signal from sender is emited
*/
void DataControl::canSyncSlot(){
    QObject * tmr = sender();
    for (uint i=0; i< canSyncTimers.size(); i++){
        if (canSyncTimers.at(i) == tmr) {
            canItem item = canSyncs.at(i);
            emit sendCANDataByte(item.address,item.data,item.bytes);
        }
    }
}

/*
* error control method that is not used anymore
*/
void DataControl::receive_sensor_data(meta * sensor){
    try{
        receiveData(sensor);
    } catch (...) {
        pushMessage("CRITICAL ERROR: Crash on receiving sensor data");
    }
}

/**
 * @brief DataControl::receive_can_data : gets data from canbus
 * @param addr : CAN address
 * @param data : data transmitted
 */
void DataControl::receive_can_data(uint32_t addr, uint64_t data){
    try{
        bool print = false;
        string msg;
        //check whether address matches any state machine address
        for (uint i = 0; i < FSMs.size(); i++){
            statemachine * currFSM = FSMs.at(i);
            QCoreApplication::processEvents();
            if (currFSM->primAddress == static_cast<int>(addr)){
                for (uint j = 0; j < currFSM->states.size(); j++){
                    QCoreApplication::processEvents();
                    system_state * currState = currFSM->states.at(j);
                    if(currState->value == static_cast<int>(isolateData64(static_cast<uint>(currState->auxAddress),static_cast<uint>(currState->offset),data))){
                        change_system_state(currState);
                    } else if (currState->active){
                        deactivateLog(currState);
                    }
                }
                if (print){
                    emit pushMessage(msg);
                }
                emit updateFSM(currFSM);
            }
        }

        //check whether address matches any status address
        for (uint i = 0; i < states.size(); i++){
            QCoreApplication::processEvents();
            if(states.at(i)->primAddress == addr && states.at(i)->value == isolateData64(states.at(i)->auxAddress,states.at(i)->offset,data)){
                change_system_state(states.at(i));
            } else if (states.at(i)->primAddress == static_cast<int>(addr)){
                emit deactivateState(states.at(i));
            }
        }

        //check whether address matches any sensor address
        if ( canSensorGroup.find(addr) == canSensorGroup.end() ) {
            // not found
        } else {
            vector<meta *>* specSensors = canSensorGroup.at(addr);
            for (uint i = 0; i < specSensors->size(); i++){
                QCoreApplication::processEvents();
                meta * currSensor = specSensors->at(i);
                double val;
                //compares sensor auxillary address to first 4 bytes of received packet
                // if true sets sensor value
                if(currSensor->motor && static_cast<uint16_t>(data>>40)==currSensor->auxAddress){
                    val=static_cast<uint8_t>(data>>currSensor->offset);
                    //if(int(val)==int(currSensor->val))
                    currSensor->val = val;
                }
                else if(!currSensor->motor){//tsi packets that need to be edited
                    val=static_cast<int>(isolateData64(currSensor->auxAddress,currSensor->offset,data));
                    //if(int(val)==int(currSensor->val))
                    currSensor->val = val;
                }
                receiveData(currSensor);
                QCoreApplication::processEvents();
            }
        }
    } catch (...) {
        cout << "CRITICAL ERROR: Crash on receiving CAN data" << endl;
        pushMessage("CRITICAL ERROR: Crash on receiving CAN data");
    }
}

/**
 * @brief DataControl::isolateData64 isolate bits of data as specified
 * @param auxAddress : starting bit - LSB -> 63, MSB -> 0
 * @param offset : number of bits in field
 * @param data : bitstream (64) to be isolated
 * @return 32 bit result
 */
uint32_t DataControl::isolateData64(uint auxAddress, uint offset, uint64_t data){
    if (auxAddress > 63 || offset > 64) return 0;
    uint lastAddr = sizeof (data)*8 - offset;
    data = data << auxAddress;
    data = data >> lastAddr;
    return static_cast<uint32_t>(data);
}
/**
 * @brief DataControl::LSBto64Spec shift specified LSBits to specified field
 * @param auxAddress : starting bit of target field - LSB -> 63, MSB -> 0
 * @param offset : number of bits in field
 * @param data : bitstream (64) to be isolated
 * @return 64 bit result
 */
uint64_t DataControl::LSBto64Spec(uint auxAddress, uint offset, uint64_t data){
    if (auxAddress > 63 || offset > 64) return 0;
    uint lastAddr = sizeof (data)*8 - offset;
    uint firstAddr = sizeof (data)*8 - auxAddress;
    data = data << lastAddr;
    data = data >> firstAddr;
    return data;
}

/**
 * @brief DataControl::receiveData : receive sensor data
 * @param currSensor
 */
void DataControl::receiveData(meta * currSensor){
    calibrateData(currSensor);
    checkThresholds(currSensor);
    emit updateDisplay(currSensor);
}
/**
 * @brief DataControl::logData - records specified sensor data in the respective database
 * @param currSensor
 */
void DataControl::logData(meta *currsensor){
    dbase->add_row_sec("sensor_data","id,time,sensor_value",
                       "'" + to_string(currsensor->sensorIndex) + "',"+"'" + getProgramTime()+"','" + to_string(currsensor->calVal) + currsensor->unit+"'");
}

/**
 * @brief DataControl::checkThresholds - check whether specified sensor data exceeds configured thresholds
 * @param sensor
 */
void DataControl::checkThresholds(meta * sensor){
    string msg;
    if (sensor->calVal >= sensor->maximum){
        if (sensor->state != 1){
            sensor->state = 1;
            emit updateEditColor("red",sensor);
            for (auto const &x : sensor->groups){
                if (!subsystemMap[x]->error){
                    subsystemMap[x]->error = true;
                    emit subsystemMap[x]->updateHealth();
                }
            }
            msg = sensor->sensorName + " exceeded upper threshold: " + to_string(sensor->maximum);
            emit pushMessage(msg);
            logData(sensor);
            executeRxn(sensor->respnum);
            pushMessage(msg);
        }
    } else if (sensor->calVal <= sensor->minimum){
        if (sensor->state != -1){
            sensor->state = -1;
            emit updateEditColor("blue",sensor);
            for (auto const &x : sensor->groups){
                if (!subsystemMap[x]->error){
                    subsystemMap[x]->error = true;
                    emit subsystemMap[x]->updateHealth();
                }
            }
            msg = sensor->sensorName + " below lower threshold: " + to_string(sensor->minimum);
            emit pushMessage(msg);
            logData(sensor);
            executeRxn(sensor->respnum);
            pushMessage(msg);
        }
    } else {
        if (sensor->state != 0){
            sensor->state = 0;
            emit updateEditColor("yellow",sensor);
            for (auto const &x : sensor->groups){
                if (subsystemMap[x]->error) {
                    subsystemMap[x]->checkError();
                    emit subsystemMap[x]->updateHealth();
                }
            }
        }
    }
}

/**
 * @brief DataControl::calibrateData : calibrates sensor data
 * @param currSensor
 */
void DataControl::calibrateData(meta * currSensor){
    double val=currSensor->calVal;
    currSensor->calData();
    currSensor->calRec();
    double data = 0;
    vector<poly> pol = currSensor->calPolynomial;
    if (pol.size() > 0){
        for (uint i = 0; i < pol.size(); i++){
            data += pol.at(i).coefficient*pow(currSensor->val,pol.at(i).exponent);
        }
        currSensor->calVal = data;
    }
//    if(val+currSensor->trigval<currSensor->calVal||val-currSensor->trigval>currSensor->calVal){
//        currSensor->trigval=currSensor->trigger;
//        //logData(currSensor);
//    }else{
//        currSensor->trigval=currSensor->trigval+val-currSensor->calVal;
//    }
}



/**
 * @brief DataControl::change_system_state changes state as specified
 * @param newState : state to be activated
 * @return 1 on success, 0 otherwise
 */
int DataControl::change_system_state(system_state * newState){
    try{
        newState->active = true;
        string colString = "time,state,message";
        string rowString = "'" + getProgramTime() + "','" + newState->name + "','Entered State'";

        //change state of system
        currState = newState->name;
        //display change on back and front screen
        emit activateState(newState);
        pushMessage(rowString);
        return 1;
    } catch(...){
        return 0;
    }
}

/**
 * @brief DataControl::deactivateLog records state-exitting to database
 * @param prevstate : state being exitted
 */
void DataControl::deactivateLog(system_state *prevstate){
    prevstate->active = false;
    string colString = "time,state,message";
    string rowString = "'" + getProgramTime() + "','" + prevstate->name + "','Exit State'";
    pushMessage(rowString);
}

/**
 * @brief DataControl::executeRxn executes response of specified index
 * @param responseIndex : response identifier
 */
void DataControl::executeRxn(int responseIndex){
    //print to logpushMessage
    try{
        response rsp = responseMap[responseIndex];
        if (rsp.address >= 0){
            emit sendCANData(rsp.address,rsp.data);
        }
    } catch (...) {
        pushMessage("CRITICAL ERROR: Crash on executing reaction to data");
    }
}

/**
 * @brief DataControl::get_curr_time - retrieves current operation system time
 * @return
 */
string DataControl::get_curr_time(){
    time_t t = time(nullptr);
    struct tm now = *localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf),"%D_%T",&now);
    return buf;
}

string DataControl::get_curr_date(){
    time_t t = time(nullptr);
    struct tm now = *localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf),"%D",&now);
    remove(std::begin(buf), std::end(buf), '/');
    return buf;
}

/** something stupid
 * @brief Function to remove all spaces from a given string
 */
string DataControl::removeSpaces(string &str)
{
    int size = str.length();
    for(int j = 0; j<=size; j++){
        for(int i = 0; i <=j; i++){
            if(str[i] == ' ') str.erase(str.begin() + i);
        }
    }
    return str;
}
