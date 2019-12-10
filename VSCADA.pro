TEMPLATE = app
CONFIG += console c++11
CONFIG += app_bundle
CONFIG += qt

TARGET = VSCADA
QT     += xml
QT     += core gui printsupport sql
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$_PRO_FILE_PWD_
DEFINES += MAIN_QML=\\\"Basic.qml\\\"

LIBS += -pthread
LIBS += -lusb-1.0
LIBS += -lsqlite3

#LIBS += -l:libudev.so.1
#LIBS += -l:libc.so.6
#LIBS += -l:libusb-1.0.so.0
#LIBS += -l:ld-linux.so.3

SOURCES += \
    dbtable.cpp \
        main.cpp \
    datacontrol.cpp \
    config.cpp \
    mainwindow.cpp\
    qcustomplot.cpp \
    canbus_interface.cpp \
    gpio_interface.cpp \
    detailpage.cpp \
    qcgaugewidget.cpp \
    libusb_interface/pmd.c \
    libusb_interface/test-usb7204.c \
    libusb_interface/usb-7204.c \
    usb7402_interface.cpp \
    libusb_interface/hidapi.c \
    traffictest.cpp \
    group.cpp \
    watchdog.cpp \

HEADERS += \
    datacontrol.h \
    dbtable.h \
    typedefs.h \
    config.h \
    mainwindow.h\
    qcustomplot.h\
    typedefs.h \
    canbus_interface.h \
    gpio_interface.h \
    detailpage.h \
    qcgaugewidget.h \
    libusb_interface/pmd.h \
    libusb_interface/usb-7204.h \
    libusb_interface/test-usb7204.h \
    usb7402_interface.h \
    libusb_interface/hidapi.h \
    traffictest.h \
    group.h \
    watchdog.h \
    sqlite3.h

FORMS += \
    mainwindow.ui \
    detailpage.ui \

QT += serialbus widgets
QT += core
QMAKE_CXXFLAGS += -std=gnu++0x -pthread
QMAKE_CFLAGS += -std=gnu++0x -pthread

# Default rules for deployment.
target.path=/home/pi
INSTALLS += target

DISTFILES += \
    0
