/********************************************************************************
** Form generated from reading UI file 'detailpage.ui'
**
** Created by: Qt User Interface Compiler version 6.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DETAILPAGE_H
#define UI_DETAILPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_detailPage
{
public:
    QMenuBar *menubar;
    QWidget *centralwidget;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *detailPage)
    {
        if (detailPage->objectName().isEmpty())
            detailPage->setObjectName(QString::fromUtf8("detailPage"));
        detailPage->resize(800, 600);
        menubar = new QMenuBar(detailPage);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        detailPage->setMenuBar(menubar);
        centralwidget = new QWidget(detailPage);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        detailPage->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(detailPage);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        detailPage->setStatusBar(statusbar);

        retranslateUi(detailPage);

        QMetaObject::connectSlotsByName(detailPage);
    } // setupUi

    void retranslateUi(QMainWindow *detailPage)
    {
        detailPage->setWindowTitle(QCoreApplication::translate("detailPage", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class detailPage: public Ui_detailPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DETAILPAGE_H
