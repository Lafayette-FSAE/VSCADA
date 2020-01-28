#include "gpio_interface.h"

/**
 * @brief gpio_interface::gpio_interface : class constructor
 * @param gpioSen : configured GPIO sensors
 * @param i2cSen : configured I2C sensors
 * @param responses : configured responses
 */
gpio_interface::gpio_interface(vector<meta *> gpioSen, vector<meta *> i2cSen, vector<response> responses)
{
    timer = new QTimer;
    i2cSensors = i2cSen;
    gpioSensors = gpioSen;
    responseVector = responses;

    for (uint i = 0; i < gpioSensors.size(); i++){
        meta * currSensor = gpioSensors.at(i);
        if (currSensor->gpioPin >= 0){
            GPIOExport(currSensor->gpioPin);
            GPIODirection(currSensor->gpioPin,IN);
        }
    }

    for (uint i = 0; i < i2cSensors.size(); i++){
        meta * currSensor = i2cSensors.at(i);
        int fd;
        const char * filename = "/dev/i2c-1";
        if ((fd = open(filename, O_RDWR)) < 0)
        {
            printf("Failed to open the i2c bus\n");
        } else if (ioctl(fd, I2C_SLAVE, currSensor->i2cAddress) < 0)
        {
            printf("Failed to acquire bus access and/or talk to slave.\n");
        }

        //write configuration stream
        for (uint j = 0; j < currSensor->i2cConfigs.size(); j++){
            uint32_t data = currSensor->i2cConfigs.at(j);
            char configData[3] = {0};
            configData[0] =  static_cast<char>(data);
            configData[1] =  static_cast<char>(data >> 8);
            configData[2] =  static_cast<char>(data >> 16);
            if (write(fd, configData,3) != 3){
                std::cout << "Writing i2c config address pointer for " << currSensor->sensorName << " failed" << endl;
            }
        }
    }

    for (uint i = 0; i < responseVector.size(); i++){
        response currRsp = responseVector.at(i);
        if (currRsp.gpioPin >= 0 && currRsp.gpioPin != 21){
            GPIOExport(currRsp.gpioPin);
            GPIODirection(currRsp.gpioPin,OUT);
        }
    }

    connect(timer, SIGNAL(timeout()), this, SLOT(gpioCheckTasks()));
}

/**
 * @brief gpio_interface::~gpio_interface : class destructor
 */
gpio_interface::~gpio_interface(){
    for (uint i = 0; i < gpioSensors.size(); i++){
        meta * currSensor = gpioSensors.at(i);
        if (currSensor->gpioPin >= 0){
            GPIOUnexport(currSensor->gpioPin);
        }
    }
    for (uint i = 0; i < responseVector.size(); i++){
        response currRsp = responseVector.at(i);
        if (currRsp.gpioPin >= 0){
            GPIOUnexport(currRsp.gpioPin);
        }
    }
}

/**
 * @brief GPIOExport - enables GPIO pin for read/write
 * @param pin
 * @return
 */
int gpio_interface::GPIOExport(int pin)
{
    #define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, static_cast<size_t>(bytes_written));
    close(fd);
    return(0);
}

/**
 * @brief GPIOUnexport - disables GPIO pin for read/write
 * @param pin
 * @return
 */
int gpio_interface::GPIOUnexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, static_cast<size_t>(bytes_written));
    close(fd);
    return(0);
}

/**
 * @brief GPIODirection - sets input/output direction
 * @param pin
 * @param dir
 * @return
 */
int gpio_interface::GPIODirection(int pin, int dir)
{
    static const char s_directions_str[]  = "in\0out";

    #define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    usleep(1000);
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
    }

    cout << path << endl;
    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

/**
 * @brief GPIORead - reads GPIO pin
 * @param pin
 * @return 0 upon success, -1 otherwise
 */
int gpio_interface::GPIORead(meta * sensor)
{
    #define VALUE_MAX 30
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", sensor->gpioPin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return(-1);
    }

    if (-1 == read(fd, value_str, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        return(-1);
    }

    close(fd);
    sensor->val = atoi(value_str);
    return 0;
}

/**
 * @brief GPIOWrite - writes to GPIO pin
 * @param pin
 * @param value
 * @return
 */
void gpio_interface::GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value!\n");
    }

    close(fd);
}

/**
 * @brief gpio_interface::gpioCheckTasks : collect configured GPIO data
 */
void gpio_interface::gpioCheckTasks(){
    cout << "Checking GPIO sensors" << endl;
    for (uint i = 0; i < gpioSensors.size(); i++){
        if (GPIORead(gpioSensors.at(i)) == 0){
            emit sensorValueChanged(gpioSensors.at(i));
        } else {
            std::cout << "Sensor read for gpio sensor " << i2cSensors.at(i)->sensorName << " failed" << endl;
        }
    }

    for (uint i = 0; i < i2cSensors.size(); i++){
        if (i2cRead(i2cSensors.at(i)) == 0){
            emit sensorValueChanged(i2cSensors.at(i));
        } else {
            //std::cout << "Sensor read for i2c sensor " << i2cSensors.at(i)->sensorName << " failed" << endl;
        }
    }
}

/**
 * @brief gpio_interface::setSamplingRate : sets sampling rate
 * @param newRate : rate to be set
 */
void gpio_interface::setSamplingRate(int newRate){
    samplingRate = newRate;
    stopGPIOCheck();
    startGPIOCheck();
}

/**
 * @brief gpio_interface::startGPIOCheck : starts sampling timer
 */
void gpio_interface::startGPIOCheck(){
    timer->start(samplingRate);
}

/**
 * @brief gpio_interface::stopGPIOCheck : stops sampling timer
 */
void gpio_interface::stopGPIOCheck(){
    timer->stop();
}

/**
 * @brief gpio_interface::i2cRead : reads the I2C bus on the specified address
 * @param sensor : I2C sensor
 * @return : 0 on success, -1 otherwise
 */
int gpio_interface::i2cRead(meta * sensor){
    int fd;
    const char * filename = "/dev/i2c-1";
    if ((fd = open(filename, O_RDWR)) < 0)
    {
        printf("Failed to open the i2c bus\n");
    } else if (ioctl(fd, I2C_SLAVE, sensor->i2cAddress) < 0)
    {
        printf("Failed to acquire bus access and/or talk to slave.\n");
    }

    //write read pointer/register
    char pointerBuf[1] = {0};
    pointerBuf[0] = static_cast<char>(sensor->i2cReadPointer);
    if (write(fd,pointerBuf,1) != 1){
        //std::cout << "Writing i2c pointer for " << sensor->sensorName << " failed" << endl;
        return -1;
    }

    usleep(static_cast<__useconds_t>(sensor->i2cReadDelay));

    //read from i2c device
    char readBuf[2] = {0};
    if (read(fd, readBuf, 2) != 2){
        //std::cout << "i2c read for " << sensor->sensorName << " failed" << endl;
        return -1;
    } else {
        uint16_t result = static_cast<uint16_t>(readBuf[0]);
        result = static_cast<uint16_t>(result << 8);
        result = static_cast<uint16_t>(result | readBuf[1]);
        if (sensor->i2cDataField < 16){
            result = static_cast<uint16_t>(result << (16 - sensor->i2cDataField));
            result = static_cast<uint16_t>(result >> (16 - sensor->i2cDataField));
        }
        sensor->val = static_cast<double>(result);
        return 0;
    }
}

/**
 * @brief gpio_interface::i2cWrite : writes to I2C bus
 * @param address : slave address
 * @param data : data to write to slave
 * @return : 0 on success, -1 otherwise
 */
int gpio_interface::i2cWrite(int address, int data){
    int file_i2c;
    long length = 4;
    unsigned char buffer[60] = {0};
    unsigned char * writeData = nullptr;
    memcpy(writeData, &data, sizeof (data));

    //----- OPEN THE I2C BUS -----//
    const char * filename = "/dev/i2c-1";
    if ((file_i2c = open(filename, O_WRONLY)) < 0)
    {
        printf("Failed to open the i2c bus");
        return -1;
    }

    if (ioctl(file_i2c, I2C_SLAVE, address) < 0)
    {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        return -1;
    }

    //----- WRITE BYTES -----//
    //write() returns the number of bytes actually written,
    //if it doesn't match then an error occurred (e.g. no response from the device)
    if (write(file_i2c, buffer, static_cast<unsigned long>(length)) != length)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("Failed to write to the i2c bus.\n");
        return -1;
    }
    return 0;
}
