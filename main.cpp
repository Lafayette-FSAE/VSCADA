#include "mainwindow.h"
#include <QApplication>
#include "typedefs.h"
#include "config.h"
#include "watchdog.h"

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0){
        Watchdog * watchdog = new Watchdog(5000,getppid());
    }

    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("fusion"));

    QPalette palette;
    //palette.setColor(QPalette::Window, QColor(20,30,150));
    palette.setColor(QPalette::Window, QColor(20,180,100));
    palette.setColor(QPalette::WindowText, QColor(255,255,255));
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(180,0,130));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::yellow);
    palette.setColor(QPalette::Button, QColor(0,0,75));
    palette.setColor(QPalette::ButtonText, QColor(255,255,255));
    palette.setColor(QPalette::BrightText, Qt::red);

    palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    palette.setColor(QPalette::HighlightedText, Qt::black);

    QApplication::setPalette(palette);

    MainWindow * w = new MainWindow;
//    dashboard * d= new dashboard;

    w->showFullScreen();
    return a.exec();
}
