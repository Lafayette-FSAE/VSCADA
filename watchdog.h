#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <QObject>
#include <QTimer>
#include <signal.h>
#include "typedefs.h"

using namespace std;

class Watchdog: public QObject
{
    Q_OBJECT
public:
    Watchdog(int tout_ms, pid_t ppid);
    ~Watchdog();
    void timerStatus();

    pid_t parent_pid;
    int timeout = 0;
    int tmr;
    QTimer * timer = new QTimer;

public slots:
    void timeOut();
    void restartTimer();

signals:
    void feedWatchDog();

};

#endif // WATCHDOG_H
