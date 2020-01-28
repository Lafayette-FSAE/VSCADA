#include "canbus_interface.h"

/**
 * @brief canbus_interface::canbus_interface class constructor
 */
canbus_interface::canbus_interface(int canRate, vector<meta*> sensorVector) {
    bitrate = canRate;

    can_bus = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),QStringLiteral("can0"),&errmsg);
    QList<QCanBusDevice::Filter> filterList;

    for (uint i = 0; i < sensorVector.size(); i++){
            QCanBusDevice::Filter filter;
            filter.frameId = sensorVector.at(i)->primAddress;
            filter.frameIdMask = 0xFFFFu;
            filter.format = QCanBusDevice::Filter::MatchBaseFormat;
            filter.type = QCanBusFrame::InvalidFrame;
            filterList.append(filter);
    }
    can_bus->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(filterList));

    canconnect();
    rebootCAN();
}

/**
 * @brief canbus_interface::~canbus_interface class destructor
 */
canbus_interface::~canbus_interface()
{
    if (can_bus != nullptr) delete can_bus;

}

/**
 * @brief canbus_interface::canconnect establishes connection with canbus shield
 * @return true on success, false otherwise
 */
bool canbus_interface::canconnect() {
    if (!can_bus) {
        qDebug() << "Error creating device" << endl;
        qDebug() << errmsg << endl;
        return false;
    } else {
        bool connectflag = can_bus->connectDevice();
        if (connectflag) {
            qDebug() << "Connected!" << endl;
            return true;
        } else {
            errmsg = can_bus->errorString();
            qDebug() << "Error connecting device." << endl;
            qDebug() << errmsg << endl;
            return false;
        }
    }
}

/**
 * @brief canbus_interface::recieve_frame signal triggered when CAN frame is received
 */
void canbus_interface::recieve_frame() {
    try {
        if(canMutex.try_lock()){
            while (can_bus->framesAvailable() && can_bus->framesAvailable() <= CAN_FRAME_LIMIT){
                QCanBusFrame recframe = can_bus->readFrame();
                QByteArray b = recframe.payload();
                uint32_t a = recframe.frameId();

                int64_t data = 0;
                //qDebug() << "CAN address: " << a << ", QByteArray: " << b << endl;
                for (int i = 0; i < b.size(); i++){
                    data = data + ((static_cast<uint64_t>(b[i]) & 0xFF) << ((b.size()-1)*8 - i*8));
                }
                //qDebug() << "CAN address: " << a << ", QByteArray: " << data << endl;
                QCoreApplication::processEvents();
                emit process_can_data(a,data);
            }

            if (can_bus->framesAvailable() > CAN_FRAME_LIMIT){
                while (can_bus->framesAvailable()) {
                    QCoreApplication::processEvents();
                    can_bus->readFrame();
                }
                pushMsg("Frame Buffer Clogged... Frames Dumped");
            }
            canMutex.unlock();
        }
    } catch (...) {
        pushMsg("CRITICAL ERROR: Crash on receiver_frame message");
    }
}

/**
 * @brief canbus_interface::sendData sends 64 bit data over CAN to specified address
 * @param addr CAN address
 * @param data : data to be sent
 */
void canbus_interface::sendData(int addr, uint64_t data){
    try{
        QByteArray byteArr;
        char * charData = static_cast<char*>(static_cast<void*>(&data));
        for(int i = sizeof(data)-1; i >= 0; i--){
            byteArr.append(charData[i]);
        }
        //cout << "Send data Address: " << addr  << " Value: " << data << endl;
        QCanBusFrame * outFrame = new QCanBusFrame;
        outFrame->setFrameId(static_cast<quint32>(addr));
        outFrame->setPayload(byteArr);
        can_bus->writeFrame(*outFrame);

        delete outFrame;
    } catch (...) {
        pushMsg("CRITICAL ERROR: Crash on sending CAN data");
    }
}

/**
 * @brief canbus_interface::sendDataByte sends specified bit data over CAN to specified address
 * @param addr CAN address
 * @param data : data to be sent
 * @param byte : number of bytes in packet
 */
void canbus_interface::sendDataByte(int addr, uint64_t data, int bytes){
    try{
        QByteArray byteArr;
        char * charData = static_cast<char*>(static_cast<void*>(&data));
        for(int i = bytes-1; i >= 0; i--){
            byteArr.append(charData[i]);
        }
        if(bytes<8)
            while(byteArr.size()<8){
                byteArr.append(char(0));
        }
        //cout << "Address: " << addr  << " Value: " << data << " Bytes "<< byteArr.size() <<endl;
        QCanBusFrame * outFrame = new QCanBusFrame;
        outFrame->setFrameId(static_cast<quint32>(addr));
        outFrame->setPayload(byteArr);
        can_bus->writeFrame(*outFrame);

        delete outFrame;
    } catch (...) {
        pushMsg("CRITICAL ERROR: Crash on sending CAN data");
    }
}

/**
 * @brief canbus_interface::rebootCAN resets CAN network and sets bitrate
 */
void canbus_interface::rebootCAN(){
    try{
        system("sudo ip link set can0 down");
        std::string s = "sudo ip link set can0 up type can bitrate " + std::to_string(bitrate);
        const char * command = s.c_str();
        system(command);
        if (canconnect()){
            emit pushMsg("CAN boot successful");
        } else {
            emit pushMsg("ERROR: CAN boot failed");
        }
    } catch (...) {
        pushMsg("CRITICAL ERROR: Crash on CAN reboot");
    }
}

/*
* allows system to use can packets when recieved
*/
void canbus_interface::enableCAN(){
    connect(can_bus, &QCanBusDevice::framesReceived, this, &canbus_interface::recieve_frame);
}
