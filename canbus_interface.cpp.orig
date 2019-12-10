#include "canbus_interface.h"

canbus_interface::canbus_interface(std::vector<meta> sensorVector, std::string modulename) {
    this->modulename = modulename;
    std::vector<meta> sensorVec = sensorVector;
    can_bus = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),QStringLiteral("can0"),&errmsg);
    canconnect();
    connect(can_bus, &QCanBusDevice::framesReceived, this, &canbus_interface::recieve_frame);

        datapoint dpt;
    int a = sensorVector.size();
    for(int i = 0; i < (int)sensorVector.size(); i++){
        dpt.displayed = 0;
        dpt.monitored = 0;
        dpt.sensorIndex = sensorVector.back().sensorIndex;
        dpt.canAddress = sensorVector.back().canAddress;
        dpt.gpioPin = sensorVector.back().gpioPin;
        sensorVector.pop_back();
        dpt.value = 0;
        dpa.push_back(dpt);
    }
}

canbus_interface::~canbus_interface()
{
    delete can_bus;
}


bool canbus_interface::canconnect() {
    if (!can_bus) {
        qDebug() << "Error creating device" << endl;
        qDebug() << errmsg << endl;
//        ui->textOut->append(errmsg);
        return false;
    }
    else {
        bool connectflag = can_bus->connectDevice();
        if (connectflag) {
            qDebug() << "Connected!" << endl;
            return true;
        }
        else {
            errmsg = can_bus->errorString();
            qDebug() << "Error connectiong device." << endl;
            qDebug() << errmsg << endl;
//            ui->textOut->append(errmsg);
            return false;
        }
    }

}



void canbus_interface::recieve_frame() {
    qDebug() << "revieve_frame called" << endl;
    QCanBusFrame recframe = can_bus->readFrame();
    QByteArray a = recframe.payload();

    if (dpa.empty()) {
        qDebug() << "Datapoint array has no item" <<endl;
        return;
    }

    for (std::vector<datapoint>::iterator it = dpa.begin(); it != dpa.end(); ++it) {
        if(it.base()->canAddress == recframe.frameId()) {
            int data = 0;
            for (char i:a) {
                data = data + (int)i;
            }

            datapoint dp = *it;
            dp.value = data;

            *it = dp;
            break;
        }

        if (it+1 == dpa.end()) {
//            qDebug() << "SensorIndex does not match configuration file" << endl;;
            return;
        }
    }

    qDebug() << "frame recieved" <<endl;
    qDebug() << QString::number(getdatapoint_canadd(recframe.frameId()).value) <<endl;
}

datapoint canbus_interface::getdatapoint(uint32_t index) { // return a datapoint with certain index. an empty datapoint will be returned if no such index
    for (datapoint d:dpa) {
        if (d.sensorIndex == index) {
            return d;
        }
    }
    datapoint edp;
    edp.sensorIndex = -1;
    return edp;
}

datapoint canbus_interface::getdatapoint_canadd(uint32_t canadd) { // return a datapoint with certain index. an empty datapoint will be returned if no such index
    for (datapoint d:dpa) {
        if (d.canAddress == canadd) {
            return d;
        }
    }
    datapoint edp;
    edp.sensorIndex = -1;
    return edp;
}

std::string canbus_interface::get_curr_time(){
    time_t t = time(0);
    struct tm now = *localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf),"%X",&now);
    return buf;
}



