#include "watchdog.h"

Watchdog::Watchdog(int tout_ms, pid_t ppid){
    parent_pid = ppid;
    timeout = tout_ms;
    cout << "WatchDog configured and started" << endl;
    tmr = WATCHDOG_PERIOD;
    int currPid = getpid();
    string pidStream = "echo " + to_string(currPid) + " > watchdogpid.txt";
    cout << "Watchdog PID write cmd: " << pidStream << endl;
    system(pidStream.c_str());
    timerStatus();
}

Watchdog::~Watchdog(){
}

void Watchdog::restartTimer(){
    tmr = WATCHDOG_PERIOD;
}

void Watchdog::timerStatus(){
    int fileData = 0;
    while(1){
        fstream  wdfile;
        wdfile.open("watchdog.txt", ios::out | ios::in );
        wdfile >> fileData;
        wdfile.close();
        if (fileData){
            if (tmr == 0){
                cout << "Session timed out. Initiating reboot..." << endl;
                timeOut();
                return;
            }
            sleep(1);
            tmr--;
        } else {
            cout << "Session Running... Restarting wdog timer..." << endl;
            restartTimer();
            system("echo 1 > watchdog.txt");
        }
    }
}

void Watchdog::timeOut(){
    cout << "Killing Parent PID: " << parent_pid << endl;
    kill(parent_pid, SIGKILL);
    cout << "Delay 10s..." << endl;
    sleep(10);
    cout << "Launching new instance of parent program..." << endl;
    system("./VSCADA");
    cout << "Killing watchdog..." << endl;
    kill(getpid(),SIGKILL);
}
