#include "config.h"

/**
 * @brief Config::Config class constructor
 */
Config::Config(){
    //blah...
}

/**
 * @brief Config::~Config class destructor
 */
Config::~Config(){
    if (dataCtrl != nullptr) delete dataCtrl;
    if (usb7204 != nullptr) delete usb7204;
    if (gpioInterface != nullptr) delete gpioInterface;
    if (canInterface != nullptr) delete canInterface;

    for (auto const& x : sensorMap){
        if (x.second != nullptr) delete x.second;
    }

    for (uint i = 0; i < FSMs.size(); i++){
        if (FSMs.at(i) != nullptr) delete FSMs.at(i);
    }

    for (uint i = 0; i < sysStates.size(); i++){
        if (sysStates.at(i) != nullptr) delete sysStates.at(i);
    }
}

/**
 * @brief Config::read_config_file_data : reads configuration file
 *  and stores all configured data
 * @return
 */
bool Config::read_config_file_data(){
    //local declarations
    vector<vector<meta *>> sensorVector;
    vector<meta> allSensors;
    vector<response> allResponses;
    vector<int> minrates;
    //vector<logic *> logicVector;
    vector<meta *> dependentSensors;
    gpioRate = 1000;
    usb7204Rate = 1000;
    canRate = 125000;

    //************************************//
    //*****extract file to DOM object*****//
    //************************************//

    qDebug("Inside the PARSE Slot");
    QDomDocument doc;
    QFile f("config.xml");
    if(!f.open(QIODevice::ReadOnly))
    {
        qDebug("Error While Reading the File");
    }

    doc.setContent(&f);
    f.close();
    qDebug("File was closed Successfully");


    //************************************************//
    //*****extract individual types from xml file*****//
    //************************************************//

    QDomNodeList mode = doc.elementsByTagName("mode");
    QDomNodeList gpioConfRate = doc.elementsByTagName("gpiorate");
    QDomNodeList canConfRate = doc.elementsByTagName("canrate");
    QDomNodeList usb7204ConfRate = doc.elementsByTagName("usb7204rate");
    QDomNodeList responseNodes = doc.elementsByTagName("response");
    QDomNodeList groupNodes = doc.elementsByTagName("group");
    QDomNodeList systemStatuses = doc.elementsByTagName("systemstatus");
    QDomNodeList stateMachines = doc.elementsByTagName("statemachine");
    QDomNodeList sensorNodes = doc.elementsByTagName("sensor");
    QDomNodeList cansync = doc.elementsByTagName("cansync");


#ifdef CONFIG_PRINT
    cout << "Number of responses: " << responseNodes.size() << endl;
    cout << "Number of sensor groupings: " << groupNodes.size() << endl;
    cout << "Number of system states: " << systemStatuses.size() << endl;
    cout << "Number of state machines: " << stateMachines.size() << endl;
    cout << "Number of configured sensors: " << sensorNodes.size() << endl;
    cout << "Number of Can messages to send : "<< cansync.size() << endl;
#endif

    //*****************************//
    //*****set interface rates*****//
    //*****************************//
    if (gpioConfRate.size() > 0){
        gpioRate = stoi(gpioConfRate.at(0).firstChild().nodeValue().toStdString());
    }
    if (usb7204ConfRate.size() > 0){
        usb7204Rate = stoi(usb7204ConfRate.at(0).firstChild().nodeValue().toStdString());
    }
    if (canConfRate.size() > 0){
        canRate = stoi(canConfRate.at(0).firstChild().nodeValue().toStdString());
    }

#ifdef CONFIG_PRINT
    cout << "CAN Rate: " << canRate << endl;
    cout << "GPIO Rate: " << gpioRate << endl;
    cout << "USB7204 Rate: " << usb7204Rate << endl;
#endif

    //*************************//
    //*****get system mode*****//
    //*************************//

    for (int i = 0; i < mode.size(); i++){
        if (mode.at(i).firstChild().nodeValue().toStdString().compare("RUN") == 0){
            systemMode = RUN;
        } else if(mode.at(i).firstChild().nodeValue().toStdString().compare("TEST") == 0){
            systemMode = TEST;
        } else {
            systemMode = RUN;
        }
    }

#ifdef CONFIG_PRINT
    if (systemMode) cout << "System Mode: " << systemMode << ": CAR" << endl;
    else cout << "System Mode: " << systemMode << ": DYNO" << endl;
#endif

    //***************************************//
    //*****process system state machines*****//
    //***************************************//

    for (int i = 0; i < stateMachines.size(); i++){
        thisFSM = new statemachine;
        QDomNodeList machineXteristics = stateMachines.at(i).childNodes();
        for (int j = 0; j < machineXteristics.size(); j++){
            if (machineXteristics.at(j).nodeName().toStdString().compare("name") == 0){
                thisFSM->name = machineXteristics.at(j).firstChild().nodeValue().toStdString();
            } else if (machineXteristics.at(j).nodeName().toStdString().compare("primaddress") == 0){
                if (isInteger(machineXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisFSM->primAddress = stoul(machineXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: primary address not an integer");
            } else if (machineXteristics.at(j).nodeName().toStdString().compare("auxaddress") == 0){
                if (isInteger(machineXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisFSM->auxAddress = stoul(machineXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: aux address not an integer");
            } else if (machineXteristics.at(j).nodeName().toStdString().compare("offset") == 0){
                if (isInteger(machineXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisFSM->offset = stoul(machineXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: address offset not an integer");
            } else if (machineXteristics.at(j).nodeName().toStdString().compare("state") == 0){
                QDomNodeList stateXteristics = machineXteristics.at(j).childNodes();
                thisState = new system_state;
                thisState->primAddress = thisFSM->primAddress;
                thisState->auxAddress = thisFSM->auxAddress;
                thisState->offset = thisFSM->offset;
                for (int k = 0; k < stateXteristics.size(); k++){
                    if (stateXteristics.at(k).nodeName().toStdString().compare("name") == 0){
                        thisState->name = stateXteristics.at(k).firstChild().nodeValue().toStdString();
                    } else if (stateXteristics.at(k).nodeName().toStdString().compare("value") == 0){
                        if (isInteger(stateXteristics.at(k).firstChild().nodeValue().toStdString()))
                            thisState->value = stoi(stateXteristics.at(k).firstChild().nodeValue().toStdString());
                        else configErrors.push_back("CONFIG ERROR: state value not an integer");
                    }
                }
                thisFSM->states.push_back(thisState);
            }
        }
        FSMs.push_back(thisFSM);
    }

#ifdef CONFIG_PRINT
    for (uint i = 0; i < FSMs.size(); i++){
        cout << "state machine name: " << FSMs.at(i)->name << endl;
        cout << "state machine states: " << FSMs.at(i)->states.size() << endl;
        cout << "state machine address: " << FSMs.at(i)->primAddress << endl;
    }
#endif

    //*********************************//
    //*****process system statuses*****//
    //*********************************//

    for (int i = 0; i < systemStatuses.size(); i++){
        thisState = new system_state;
        QDomNodeList statusXteristics = systemStatuses.at(i).childNodes();
        for (int j = 0; j < statusXteristics.size(); j++){
            if (statusXteristics.at(j).nodeName().toStdString().compare("name") == 0){
                thisState->name = statusXteristics.at(j).firstChild().nodeValue().toStdString();
            } else if (statusXteristics.at(j).nodeName().toStdString().compare("primaddress") == 0){
                if (isInteger(statusXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisState->primAddress = stoul(statusXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: primary address not an integer");
            } else if (statusXteristics.at(j).nodeName().toStdString().compare("auxaddress") == 0){
                if (isInteger(statusXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisState->auxAddress = stoul(statusXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: aux address not an integer");
            } else if (statusXteristics.at(j).nodeName().toStdString().compare("offset") == 0){
                if (isInteger(statusXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisState->offset = stoul(statusXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: address offset not an integer");
            } else if (statusXteristics.at(j).nodeName().toStdString().compare("value") == 0){
                if (isInteger(statusXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisState->value = stoi(statusXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: state value not an integer");
            }
        }
        thisState->active = false;
        sysStates.push_back(thisState);
    }

    //**********************************//
    //*****process system responses*****//
    //**********************************//

    for (int i = 0; i < responseNodes.size(); i++){
        QDomNodeList responseXteristics = responseNodes.at(i).childNodes();
        response thisRsp;
        thisRsp.responseIndex=i;
        thisRsp.address = 1000;
        thisRsp.data = -1;
        thisRsp.offset = 0;
        thisRsp.gpioPin = -1;
        thisRsp.gpioValue = -1;
        for (int j = 0; j < responseXteristics.size(); j++){
                        if (responseXteristics.at(j).nodeName().toStdString().compare("id") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.responseIndex = stoi(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: response index not an integer");
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("description") == 0){
                thisRsp.msg = responseXteristics.at(j).firstChild().nodeValue().toStdString();
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("primaddress") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.address = stoul(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: primary address not an integer");
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("auxaddress") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.data = stoul(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: aux address not an integer");
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("offset") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.offset = stoul(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: address offset not an integer");
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("gpiopin") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.gpioPin = stoi(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: GPIO pin not an integer");
            } else if (responseXteristics.at(j).nodeName().toStdString().compare("gpioval") == 0){
                if (isInteger(responseXteristics.at(j).firstChild().nodeValue().toStdString()))
                    thisRsp.gpioValue = stoi(responseXteristics.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: GPIO response value not an integer");
            }
        }
        responseMap.insert(make_pair(i,thisRsp));
        allResponses.push_back(thisRsp);
    }

#ifdef CONFIG_PRINT
    for (uint i = 0; i < allResponses.size(); i++){
        //cout << "Response ID: " << allResponses.at(i).responseIndex << endl;
        cout << "description: " << allResponses.at(i).msg << endl;
        cout << "can prim address: " << allResponses.at(i).address << endl;
        cout << "can aux address: " << allResponses.at(i).data << endl;
        cout << "can offset: " << allResponses.at(i).offset << endl;
        cout << "gpio pin: " << allResponses.at(i).gpioPin << endl;
        cout << "gpio value: " << allResponses.at(i).gpioValue << endl << endl;
    }
#endif

    cout << "Data recording windows processed" << endl;

    //*************************//
    //*****process sensors*****//
    //*************************//


    for (int k = 0; k < sensorNodes.size(); k++){
        storedSensor = new meta;
        //storedSensor->sensorIndex=k;
        storedSensor->val = 0;
        storedSensor->calVal = 0;
        storedSensor->motor = 0;
        storedSensor->sensorIndex = -1;
        storedSensor->minimum = -1;
        storedSensor->maximum = -1;
        storedSensor->checkRate = -1;
        storedSensor->primAddress = 1000;
        storedSensor->auxAddress = 0;
        storedSensor->offset = 0;
        storedSensor->gpioPin = -1;
        storedSensor->calMultiplier = 1;
        storedSensor->calConst = 0;
        storedSensor->i2cAddress = -1;
        storedSensor->i2cReadPointer = 0;
        storedSensor->i2cReadDelay = 0;
        storedSensor->i2cDataField = 16;
        storedSensor->precision = 2;
        storedSensor->trigger=10;
        storedSensor->trigval=0;
        // init rec flag
        storedSensor->reciprocol = 0;
        QDomNodeList attributeList = sensorNodes.at(k).childNodes();
        for (int m = 0; m < attributeList.size(); m++){
            if(attributeList.at(m).nodeName().toStdString().compare("name") == 0){
                storedSensor->sensorName = attributeList.at(m).firstChild().nodeValue().toStdString();
            } else if(attributeList.at(m).nodeName().toStdString().compare("unit") == 0){
                storedSensor->unit = attributeList.at(m).firstChild().nodeValue().toStdString();
            } else if (attributeList.at(m).nodeName().toStdString().compare("id") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->sensorIndex = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor index not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("precision") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->precision = static_cast<uint8_t>(stoi(attributeList.at(m).firstChild().nodeValue().toStdString()));
                else configErrors.push_back("CONFIG ERROR: sensor precision not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("primaddress") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->primAddress = stoul(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor primary address not an integer");
                canSensors.push_back(storedSensor);
                if ( canAddressMap.find(storedSensor->primAddress) == canAddressMap.end() ) {
                    canAddressMap.insert(make_pair(storedSensor->primAddress,1));
                } else {
                    // found so skip
                }
                canSensorMap.insert(make_pair(storedSensor->primAddress+storedSensor->auxAddress, storedSensor));
                if ( canSensorGroup.find(storedSensor->primAddress) == canSensorGroup.end() ) {
                    canVectorItem =  new vector<meta*>;
                    canVectorItem->push_back(storedSensor);
                    canSensorGroup.insert(make_pair(storedSensor->primAddress, canVectorItem));
                } else {
                    uint32_t val = storedSensor->primAddress;
                    vector<meta*> * item = canSensorGroup[val];
                    item->push_back(storedSensor);
                }
            } else if (attributeList.at(m).nodeName().toStdString().compare("auxaddress") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->auxAddress = stoul(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor aux address not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("offset") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->offset = stoul(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor address offset not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("minimum") == 0){
                storedSensor->minimum = stod(attributeList.at(m).firstChild().nodeValue().toStdString());
            } else if (attributeList.at(m).nodeName().toStdString().compare("motor") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->motor = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor main field not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("maximum") == 0){
                storedSensor->maximum = stod(attributeList.at(m).firstChild().nodeValue().toStdString());
            }  else if (attributeList.at(m).nodeName().toStdString().compare("checkrate") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->checkRate = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor check rate not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("calconstant") == 0){
                storedSensor->calConst = stod(attributeList.at(m).firstChild().nodeValue().toStdString());
            } else if (attributeList.at(m).nodeName().toStdString().compare("calmultiplier") == 0){
                storedSensor->calMultiplier = stod(attributeList.at(m).firstChild().nodeValue().toStdString());
            } else if (attributeList.at(m).nodeName().toStdString().compare("gpiopin") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->gpioPin = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor GPIO pin not an integer");
                gpioSensors.push_back(storedSensor);
            } else if (attributeList.at(m).nodeName().toStdString().compare("usbchannel") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->usbChannel = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor USB channel not an integer");
                usbSensors.push_back(storedSensor);
            } else if (attributeList.at(m).nodeName().toStdString().compare("i2caddress") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->i2cAddress = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor i2c address not an integer");
                i2cSensors.push_back(storedSensor);
            } else if (attributeList.at(m).nodeName().toStdString().compare("i2creadpointer") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->i2cReadPointer = static_cast<uint8_t>(stoul(attributeList.at(m).firstChild().nodeValue().toStdString()));
                else configErrors.push_back("CONFIG ERROR: sensor i2c pointer set not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("i2cconfigdata") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->i2cConfigs.push_back(static_cast<uint32_t>(stoul(attributeList.at(m).firstChild().nodeValue().toStdString())));
                else configErrors.push_back("CONFIG ERROR: sensor i2c configuration stream not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("i2cdatafield") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->i2cDataField = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor i2c data field size not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("i2creaddelay") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->i2cReadDelay = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor i2c read delay not an integer");
            }else if (attributeList.at(m).nodeName().toStdString().compare("trigger") == 0){
                if (isInteger(attributeList.at(m).firstChild().nodeValue().toStdString()))
                    storedSensor->trigger = stoi(attributeList.at(m).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: sensor i2c read delay not an integer");
            } else if (attributeList.at(m).nodeName().toStdString().compare("calpoly") == 0){
                QDomNode polyItem = attributeList.at(m);
                QDomNodeList polyItemList = polyItem.childNodes();
                int polyCount = 0;
                for (int n = 0; n < polyItemList.size(); n++){
                    poly item;
                    if (polyItemList.at(n).nodeName().toStdString().compare("coef") == 0){
                        item.exponent = polyCount;
                        item.coefficient = stod(polyItemList.at(n).firstChild().nodeValue().toStdString());
                        polyCount++;
                    }
                    storedSensor->calPolynomial.push_back(item);
                }
            } else if (attributeList.at(m).nodeName().toStdString().compare("reciprocol") == 0) {
                // set rec flag if detected
                storedSensor->reciprocol = stod(attributeList.at(m).firstChild().nodeValue().toStdString());
            }
        }
        sensorMap.insert(make_pair(storedSensor->sensorIndex,storedSensor));
        cout << "Sensor " << sensorMap[storedSensor->sensorIndex]->sensorName << " inserted into map" << endl;
    }

    cout << "Sensors Processed" << endl;
    //**************************************//
    //*****process group member sensors*****//
    //**************************************//
    for (int i = 0; i < groupNodes.size(); i++){
        string groupId;
        int sensorId;
        vector<meta *> sensors;
        bool charc = false;
        //get group characteristics: groupId, minrate and maxrate
        QDomNodeList groupXteristics = groupNodes.at(i).childNodes();
        for (int j = 0; j < groupXteristics.size(); j++){
            if (groupXteristics.at(j).nodeName().toStdString().compare("name") == 0){
                groupId = groupXteristics.at(j).firstChild().nodeValue().toStdString();
            } else if (groupXteristics.at(j).nodeName().toStdString().compare("sensorid") == 0){
                sensorId = stoi(groupXteristics.at(j).firstChild().nodeValue().toStdString());
                if (sensorMap.count(sensorId) > 0) {
                    sensors.push_back(sensorMap[sensorId]);
                    sensorMap[sensorId]->groups.push_back(groupId);
                }
                else configErrors.push_back("CONFIG ERROR: sensor Id mismatch. Check IDs");
            }
        }
        cout << "Group Sensors Processed" << endl;

        //create group object
        grp = new Group(sensors,groupId,charc);
        groupMap.insert(make_pair(grp->groupId,grp));
    }
    //****************************************//
    //*****record all sensors to database*****//
    //****************************************//
    // write create universal tables
    DBTable *dbase = new DBTable("SCADA.db");
    dbase->create_sec("system_info","id char not null,sensorname char not null,minimum char not null,maximum char not null,calconstant char not null");

    //****************************************//
    //*****record all sensors to database*****//
    //****************************************//
    dbase->SecNum("system_info");
    sensorColString = "id,sensorname,minimum,maximum,calconstant";
    for (auto const& x : sensorMap){
        sensorRowString = "'" + to_string(x.second->sensorIndex) + "','" + x.second->sensorName + "','" + to_string(x.second->minimum)
                + "','" + to_string(x.second->maximum) + "','" + to_string(x.second->calConst) + "'";
        dbase->add_row_sec("system_info",sensorColString,sensorRowString);
    }
    for (int i = 0; i < cansync.size(); i++){
        QDomNodeList sync = cansync.at(i).childNodes();
        canItem thisItem;
        thisItem.address = 1000;
        thisItem.data = 0;
        thisItem.data2 = 0;
        thisItem.bytes = 0;
        thisItem.rate_ms = -1;
        for (int j = 0; j < sync.size(); j++){
            if (sync.at(j).nodeName().toStdString().compare("address") == 0){
                if (isInteger(sync.at(j).firstChild().nodeValue().toStdString()))
                    thisItem.address = stoul(sync.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: cansync address not an integer");
            } else if (sync.at(j).nodeName().toStdString().compare("data") == 0){
                if (isInteger(sync.at(j).firstChild().nodeValue().toStdString()))
                    thisItem.data = stoul(sync.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: cansync data not an integer");
            } else if (sync.at(j).nodeName().toStdString().compare("data2") == 0){
                if (isInteger(sync.at(j).firstChild().nodeValue().toStdString()))
                    thisItem.data2 = stoul(sync.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: cansync data not an integer");
            } else if (sync.at(j).nodeName().toStdString().compare("bytes") == 0){
                if (isInteger(sync.at(j).firstChild().nodeValue().toStdString()))
                    thisItem.bytes = stoul(sync.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: cansync bytes not an integer");
            } else if (sync.at(j).nodeName().toStdString().compare("ratems") == 0){
                if (isInteger(sync.at(j).firstChild().nodeValue().toStdString()))
                    thisItem.rate_ms = stoi(sync.at(j).firstChild().nodeValue().toStdString());
                else configErrors.push_back("CONFIG ERROR: cansync rate_ms value not an integer");
            }
        }
        canSyncs.push_back(thisItem);
    }

    //****************************************//
    //*****launch internal worker modules*****//
    //****************************************//
    usb7204 = new usb7402_interface(usbSensors);
    gpioInterface = new gpio_interface(gpioSensors,i2cSensors,allResponses);
    canInterface = new canbus_interface(canRate, canSensors);
    dataCtrl = new DataControl(gpioInterface,canInterface,usb7204,groupMap,sysStates,FSMs,
                               systemMode,responseMap,canSensorGroup,canSyncs,
                               sensorMap);
    trafficTest = new TrafficTest(canSensorMap,gpioSensors,i2cSensors,usbSensors,canRate,gpioRate,usb7204Rate,dataCtrl);

    //********************************//
    //*****initialize system info*****//
    //********************************//
    systemColString = "starttime,endtime,recordindex";
    systemRowString = "'" + get_curr_time() + "','0','0'";
    cout << "Returning from config" << endl;
    return true;
}

/**
 * @brief Config::isInteger checks whether string represents an integer
 * @param s : input string
 * @return true if input string is an integer. Otherwise returns false
 */
bool Config::isInteger(const string & s){
    char * p ;
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;
    strtol(s.c_str(), &p, 10) ;

    return (*p == 0);
}

/**
 * @brief Config::get_curr_time - retrieves current operation system time
 * @return
 */
string Config::get_curr_time(){
    time_t t = time(nullptr);
    struct tm now = *localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf),"%D_%T",&now);
    return buf;
}

/**
 * @brief Function to remove all spaces from a given string
 */
string Config::removeSpaces(string &str)
{
    int size = str.length();
    for(int j = 0; j<=size; j++){
        for(int i = 0; i <=j; i++){
            if(str[i] == ' ') str.erase(str.begin() + i);
        }
    }
    return str;
}
