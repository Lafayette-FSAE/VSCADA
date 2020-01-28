#ifndef DETAILPAGE_H
#define DETAILPAGE_H
#include <QMainWindow>
#include <QApplication>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QObject>
#include <QFrame>
#include <QRegion>
#include <QtWidgets>
#include <qcustomplot.h>
#include <string>
#include "config.h"
#include "typedefs.h"
#include "canbus_interface.h"
#include "group.h"


namespace Ui {
class detailPage;
}

class detailPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit detailPage(QWidget *parent = nullptr);
    ~detailPage();

    void setCurrentSystem(Group * subsystem);
    void setConfObject(Config * config);

    Group * currentGroup;

    QString ** systemName;

    vector<QTimer *> editTimers;
    vector<QLineEdit *> edits;
    QLineEdit * lineEdit;
    QTimer * checkTmr;
    QCheckBox *checkbox;
    QVBoxLayout * mainLayout;
    QWidget * central;

    QTabWidget * tabs;

    int unitWidth;
    int unitHeight;
    int stringSize;
    QString fontSize;
    QString editFont;

    Config * conf;

public slots:
    void updateEdits(meta * currSensor);
    void changeEditColor(string color, meta *sensor);
    void update();
    void checkTimeout();

private:
    Ui::detailPage *ui;
};

#endif

