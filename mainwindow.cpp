#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
* the UI for the system
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //initialize objects
    central = new QWidget();//first tab on the SCADA
    mainLayout = new QVBoxLayout();// the layout for the tab
    conf = new Config();// this contains everything else the software does
    timer = new QTimer();
    dbtool=new DBTable("SCADA.db");//data base for the system
    dbtool->create_sec("logs","id char not null,log string not null");//logs table
    maxSensorRow = 0;

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    central->setLayout(mainLayout);
    conf->read_config_file_data();
    cout << "Done configuring " << endl;

    // set window dimensioning parameters
    QRect rec = QApplication::desktop()->screenGeometry();
    int height=rec.height();
    int width=rec.width();
    unitWidth=width/20;
    unitHeight=height/20;
    stringSize = unitWidth/10;
    editFont = QString::number(stringSize*2);
    QRect rect = QApplication::desktop()->screenGeometry();

    //****************************************************//
    //              CREATE LOG TAB                        //
    //****************************************************//
    QScrollArea * logWidget = new QScrollArea;
    QString  butLabelFont = QString::number(stringSize*3);

    QVBoxLayout * msgLayout = new QVBoxLayout;
    QLabel * msgLabel = new QLabel;
    msgLabel->setText("System Log");
    msgLabel->setStyleSheet("font:"+butLabelFont+"pt;");
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLayout->addWidget(msgLabel, Qt::AlignCenter);

    QFont font = QFont ("Courier");
    QString startMessage = "Starting Data Acquisition...";
    QString messageFont = QString::number(stringSize*1.5);
    message = new QListWidget();
    message->setStyleSheet("font:"+messageFont+"pt;");
    message->setFixedHeight(static_cast<int>(height*0.8));
    message->setFixedWidth(static_cast<int>(width*0.98));
    message->setFont(font);
    msgLayout->addWidget(message);
    logWidget->setLayout(msgLayout);
    logMessage(startMessage);
    //****************************************************//

    // adds tabs
    QScrollArea *scrollArea = new QScrollArea();
    tabs = new QTabWidget;
    tabs->setTabsClosable(true);
    QString LabelFont = QString::number(stringSize);
    tabs->setStyleSheet("QTabBar::tab {font:"+LabelFont+"pt}");
    tabs->addTab(central,"General");
    tabs->addTab(logWidget,"System Log");
    tabs->setFixedWidth(rect.width() - 18);
    tabs->setFixedHeight(rect.height() - 50);
    QString  myfont = QString::number(stringSize*2);
    tabs->setStyleSheet("font:"+myfont+"pt;");
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(tabs);

    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    //connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(updateTab(int)));

    // create timers to identify sensor timeouts
    for (auto const& x : conf->sensorMap){
        lineEdit = new QLineEdit;
        checkTmr = new QTimer;
        connect(checkTmr, SIGNAL(timeout()), checkTmr, SLOT(stop()));
        connect(checkTmr, SIGNAL(timeout()), this, SLOT(checkTimeout()));
        checkTmr->start(x.second->checkRate);
        edits.insert(make_pair(x.second->sensorIndex,lineEdit));
        lineEdit->setStyleSheet("font: 20pt; color: #FFFF00");
        lineEdit->setAlignment(Qt::AlignCenter);
        lineEdit->setReadOnly(true);
        editTimers.insert(make_pair(x.second->sensorIndex,checkTmr));
    }

    this->setCentralWidget(scrollArea);

    update();// makes the central tab

    // connect signals to slots
    for (auto const& x : conf->groupMap){
        connect(x.second, SIGNAL(updateHealth()), this, SLOT(updateHealth()));
    }
    connect(conf->dataCtrl, SIGNAL(updateDisplay(meta *)), this, SLOT(updateEdits(meta *)));
    connect(conf->dataCtrl, SIGNAL(updateEditColor(string, meta *)), this, SLOT(changeEditColor(string, meta *)));
    connect(conf->dataCtrl, SIGNAL(deactivateState(system_state *)), this, SLOT(deactivateStateMW(system_state *)));
    connect(conf->dataCtrl, SIGNAL(activateState(system_state *)), this, SLOT(activateStateMW(system_state *)));
    connect(conf->dataCtrl, SIGNAL(updateFSM(statemachine *)), this, SLOT(updateFSM_MW(statemachine *)));
    connect(conf->canInterface, SIGNAL(pushMsg(string)), this, SLOT(receiveMsg(string)));
    connect(conf->dataCtrl, SIGNAL(pushMessage(string)), this, SLOT(receiveMsg(string)));
    connect(conf->usb7204, SIGNAL(pushMessage(string)), this, SLOT(receiveMsg(string)));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateClock()));
    connect(this, SIGNAL(sendControlValue(int, controlSpec *)), conf->dataCtrl, SLOT(receive_control_val(int, controlSpec *)));

    // display config errors
    for (uint i = 0; i < conf->configErrors.size(); i++){
        logMessage(QString::fromStdString(conf->configErrors.at(i)));
    }

    // begin data collection
    if (conf->systemMode == RUN){
        conf->canInterface->enableCAN();
        conf->gpioInterface->setSamplingRate(conf->gpioRate);
        conf->gpioInterface->startGPIOCheck();
        if (conf->usb7204->isActive) {
            conf->usb7204->setSamplingRate(conf->usb7204Rate);
            conf->usb7204->startUSBCheck();
        }
    } else if (conf->systemMode == TEST){
        conf->trafficTest->startTests();
    }
    timer->start(1000);
}

MainWindow::~MainWindow(){
    for (auto const& x : edits){
        delete x.second;
    }
    delete ui;
}

/*
* So this makes the displays that shows all the data the system collects
* any changes made here change the dino room display first tab
*/
void MainWindow::update(){

    QRect rec = QApplication::desktop()->screenGeometry();
    int screenWidth = rec.width();

    int fieldColCount = 0;
    int fieldRowCount = 0;

    for (auto const &x: conf->groupMap){
        vector<meta*> grpMeta = x.second->get_mainsensors();
        if (static_cast<int>(grpMeta.size()) > maxSensorRow) maxSensorRow = static_cast<int>(grpMeta.size());
    }

    QGridLayout * groupSectionLayout = new QGridLayout;
    for (auto const &x: conf->groupMap){
        Group * currGrp = x.second;
        vector<meta *> grpMeta = currGrp->get_mainsensors();

        QPushButton * headerLabel = new QPushButton;
        headerLabel->setFixedWidth(static_cast<int>(unitWidth*1.5));
        headerLabel->setFixedHeight(unitHeight);
        headerLabel->setText(QString::fromStdString(currGrp->groupId));
        QString  grpLabelFont = QString::number(stringSize*3);
        headerLabel->setStyleSheet("font:"+grpLabelFont+"pt;");
        headerLabel->setFixedWidth(400);
        healthButtons.insert(make_pair(currGrp->groupId,headerLabel));
        connect(headerLabel, SIGNAL(clicked()), this, SLOT(detailButtonPushed()));
        detailButtons.insert(make_pair(currGrp->groupId,headerLabel));
        //groupSectionLayout->addWidget(headerLabel,fieldRowCount,fieldColCount,1,2,Qt::AlignCenter);
        //fieldColCount=fieldColCount+2;
}
    //fieldColCount=0;
    int j=0;
    if(conf->sensorMap.size()>0){
        for (auto const &x: conf->sensorMap){
                j++;
                QLabel * label = new QLabel;
                label->setText(QString::fromStdString(x.second->sensorName));
                label->setFixedWidth(unitWidth*2);
                label->setFixedHeight(static_cast<int>(unitHeight*0.8));
                QString LabelFont = QString::number(stringSize*2);
                label->setStyleSheet("font:"+LabelFont+"pt;");
                groupSectionLayout->addWidget(label,fieldRowCount,fieldColCount);

                QLineEdit * edit = edits[x.second->sensorIndex];
                edit->setStyleSheet("font:"+editFont+"pt;");
                edit->setFixedWidth(static_cast<int>(unitWidth*2.5));
                edit->setFixedHeight(static_cast<int>(unitHeight*0.8));
                edit->setText("--");
                groupSectionLayout->addWidget(edit,fieldRowCount,fieldColCount+1);
                fieldRowCount++;
                if(j % int((conf->sensorMap.size()+2)/4)==0|conf->sensorMap.size()==j){
                    fieldRowCount = maxSensorRow + 2;
                    QFrame * hBorder1 = new QFrame(this);
                    hBorder1->setLineWidth(2);
                    hBorder1->setMidLineWidth(1);
                    hBorder1->setFrameShape(QFrame::HLine);
                    hBorder1->setFrameShadow(QFrame::Raised);
                    if (fieldColCount != 0) groupSectionLayout->addWidget(hBorder1,fieldRowCount,fieldColCount-1,1,4);
                    else groupSectionLayout->addWidget(hBorder1,fieldRowCount,fieldColCount,1,4);
                    fieldRowCount = 0;
                    fieldColCount = fieldColCount + 2;

                    QFrame * vBorder = new QFrame(this);
                    vBorder->setLineWidth(2);
                    vBorder->setMidLineWidth(1);
                    vBorder->setFrameShape(QFrame::VLine);
                    vBorder->setFrameShadow(QFrame::Raised);
                    groupSectionLayout->addWidget(vBorder,fieldRowCount,fieldColCount,maxSensorRow+3,1);
                    fieldColCount++;
                }
        }
    }
    //    for (auto const &x: conf->groupMap){
    //        Group * currGrp = x.second;
    //        vector<meta *> grpMeta = currGrp->get_mainsensors();

    //        QPushButton * headerLabel = new QPushButton;
    //        headerLabel->setFixedWidth(static_cast<int>(unitWidth*1.5));
    //        headerLabel->setFixedHeight(unitHeight);
    //        headerLabel->setText(QString::fromStdString(currGrp->groupId));
    //        QString  grpLabelFont = QString::number(stringSize*3);
    //        headerLabel->setStyleSheet("font:"+grpLabelFont+"pt;");
    //        headerLabel->setFixedWidth(400);
    //        healthButtons.insert(make_pair(currGrp->groupId,headerLabel));
    //        connect(headerLabel, SIGNAL(clicked()), this, SLOT(detailButtonPushed()));
    //        detailButtons.insert(make_pair(currGrp->groupId,headerLabel));
    //        groupSectionLayout->addWidget(headerLabel,fieldRowCount,fieldColCount,1,2,Qt::AlignCenter);
    //        fieldRowCount++;

    //        if(grpMeta.size() > 0){
    //            for (uint j = 0; j < grpMeta.size(); j++){
    //                QLabel * label = new QLabel;
    //                label->setText(QString::fromStdString(grpMeta.at(j)->sensorName));
    //                label->setFixedWidth(unitWidth*2);
    //                label->setFixedHeight(static_cast<int>(unitHeight*0.8));
    //                QString LabelFont = QString::number(stringSize*2);
    //                label->setStyleSheet("font:"+LabelFont+"pt;");
    //                groupSectionLayout->addWidget(label,fieldRowCount,fieldColCount);


    //                QLineEdit * edit = edits[grpMeta.at(j)->sensorIndex];
    //                edit->setStyleSheet("font:"+editFont+"pt;");
    //                edit->setFixedWidth(static_cast<int>(unitWidth*2.5));
    //                edit->setFixedHeight(static_cast<int>(unitHeight*0.8));
    //                edit->setText("--");
    //                groupSectionLayout->addWidget(edit,fieldRowCount,fieldColCount+1);
    //                fieldRowCount++;
    //            }
    //        }

    //        fieldRowCount = maxSensorRow + 2;

    //        QFrame * hBorder1 = new QFrame(this);
    //        hBorder1->setLineWidth(2);
    //        hBorder1->setMidLineWidth(1);
    //        hBorder1->setFrameShape(QFrame::HLine);
    //        hBorder1->setFrameShadow(QFrame::Raised);
    //        if (fieldColCount != 0) groupSectionLayout->addWidget(hBorder1,fieldRowCount,fieldColCount-1,1,4);
    //        else groupSectionLayout->addWidget(hBorder1,fieldRowCount,fieldColCount,1,4);
    //        fieldRowCount++;
    //        fieldRowCount = 0;
    //        fieldColCount = fieldColCount + 2;

    //        QFrame * vBorder = new QFrame(this);
    //        vBorder->setLineWidth(2);
    //        vBorder->setMidLineWidth(1);
    //        vBorder->setFrameShape(QFrame::VLine);
    //        vBorder->setFrameShadow(QFrame::Raised);
    //        groupSectionLayout->addWidget(vBorder,fieldRowCount,fieldColCount,maxSensorRow+3,1);
    //        fieldColCount++;
    //    }

    QWidget * groupWidget = new QWidget;
    groupWidget->setLayout(groupSectionLayout);

    QScrollArea * groupArea = new QScrollArea;
    groupArea->setFixedWidth(screenWidth-30);
    groupArea->setFixedHeight(static_cast<int>(groupWidget->height()*1.3));
    groupArea->setBackgroundRole(QPalette::Window);
    groupArea->setWidget(groupWidget);

    mainLayout->addWidget(groupArea);

    QString  butLabelFont = QString::number(stringSize);
    QString  butLabelFontLarge = QString::number(stringSize*1.5);
    QString  labelFont = QString::number(stringSize*2.5);
    QGridLayout * stateButtonLayout = new QGridLayout;
    QHBoxLayout * btnsLayout = new QHBoxLayout;
    btnsLayout->setAlignment(Qt::AlignCenter);

    int btnWidth = static_cast<int>(unitWidth*1.2);
    int btnHeight = static_cast<int>(unitHeight*1.8);
    canResetButton =new QToolButton();
    canResetButton->setText("CAN\nReset");
    canResetButton->setIconSize(QSize(btnWidth/1.6,btnHeight/1.6));
    canResetButton->setStyleSheet("font:"+butLabelFontLarge+"pt;");
    canResetButton->setFixedWidth(btnWidth);
    canResetButton->setFixedHeight(btnHeight);
    btnsLayout->addWidget(canResetButton,Qt::AlignCenter);
    QObject::connect(canResetButton, SIGNAL (clicked()), conf->canInterface , SLOT(rebootCAN()));

    usbResetButton =new QToolButton();
    usbResetButton->setText("USB7204\nReset");
    usbResetButton->setIconSize(QSize(btnWidth/1.6,btnHeight/1.6));
    usbResetButton->setStyleSheet("font:"+butLabelFontLarge+"pt;");
    usbResetButton->setFixedWidth(static_cast<int>(unitWidth*1.2));
    usbResetButton->setFixedHeight(static_cast<int>(unitHeight*1.8));
    btnsLayout->addWidget(usbResetButton,Qt::AlignCenter);
    QObject::connect(usbResetButton, SIGNAL (clicked()), conf->usb7204 , SLOT(rebootUSB7204()));

    for(uint t = 0; t < conf->FSMs.size(); t++){
        QVBoxLayout * FSMLayout = new QVBoxLayout;
        QLabel * FSMLabel = new QLabel;
        FSMLabel->setText(QString::fromStdString(conf->FSMs.at(t)->name));
        FSMLabel->setAlignment(Qt::AlignCenter);
        stateButton = new QPushButton("---");
        QPalette palplot = stateButton->palette();
        stateButton->setPalette(palplot);
        stateButton->setAutoFillBackground(true);
        //        stateButton->setStyleSheet("font:"+butLabelFont+"pt;");
        stateButton->setFixedWidth(static_cast<int>(unitWidth*2));
        stateButton->setFixedHeight(static_cast<int>(unitHeight*1.5));
        FSMButtons.push_back(stateButton);
        FSMLayout->addWidget(stateButton, Qt::AlignCenter);
        FSMLayout->addWidget(FSMLabel, Qt::AlignCenter);
        FSMLayout->setAlignment(Qt::AlignCenter);
        btnsLayout->addLayout(FSMLayout,Qt::AlignCenter);
    }

    exitButton = new QToolButton();
    exitButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QPixmap pixmap("close_btn.png");
    QIcon ButtonIcon(pixmap);
    exitButton->setIcon(ButtonIcon);
    exitButton->setIconSize(pixmap.rect().size());
    exitButton->setAutoRaise(true);
    exitButton->setStyleSheet("font:"+butLabelFont+"pt;");
    exitButton->setFixedWidth(static_cast<int>(unitWidth*1.2));
    exitButton->setFixedHeight(static_cast<int>(unitHeight*1.8));
    stateButtonLayout->addWidget(exitButton,0,2,Qt::AlignRight);
    QObject::connect(exitButton, SIGNAL (clicked()), this , SLOT(shutdownSystem()));

    stateButtonLayout->addLayout(btnsLayout,0,1,Qt::AlignCenter);
    mainLayout->addLayout(stateButtonLayout);

    QFrame * stateBorder = new QFrame(this);
    stateBorder->setLineWidth(2);
    stateBorder->setMidLineWidth(1);
    stateBorder->setFrameShape(QFrame::HLine);
    stateBorder->setFrameShadow(QFrame::Raised);
    mainLayout->addWidget(stateBorder);

    QGridLayout * statusButtonLayout = new QGridLayout;
    QHBoxLayout * statusBtnsLayout = new QHBoxLayout;
    statusBtnsLayout->setAlignment(Qt::AlignCenter);

    //    QLabel * statusLabel = new QLabel;
    //    statusLabel->setText("STATUSES: ");
    //    statusLabel->setStyleSheet("font:"+labelFont+"pt;");
    //    statusLabel->setFixedWidth(statusLabel->width());
    //    statusButtonLayout->addWidget(statusLabel,0,0,Qt::AlignLeft);

    for(uint s = 0; s < conf->sysStates.size(); s++){
        if (s != 0){
            QFrame * stateFrame = new QFrame(this);
            stateFrame->setLineWidth(2);
            stateFrame->setMidLineWidth(1);
            stateFrame->setFrameShape(QFrame::VLine);
            stateFrame->setFrameShadow(QFrame::Raised);
            statusBtnsLayout->addWidget(stateFrame, Qt::AlignCenter);
        }

        //        QFrame * stateFrame = new QFrame(this);
        //        stateFrame->setLineWidth(2);
        //        stateFrame->setMidLineWidth(1);
        //        stateFrame->setFrameShape(QFrame::VLine);
        //        stateFrame->setFrameShadow(QFrame::Raised);
        //        statusBtnsLayout->addWidget(stateFrame, Qt::AlignCenter);

        stateButton = new QPushButton();
        stateButton->setText(QString::fromStdString(conf->sysStates.at(s)->name));
        QPalette palplot = stateButton->palette();
        palplot.setColor(QPalette::Button, QColor(70,70,70));
        stateButton->setPalette(palplot);
        stateButton->setAutoFillBackground(true);
        stateButton->setStyleSheet("font:"+butLabelFont+"pt;");
        stateButton->setFixedWidth(static_cast<int>(unitWidth));
        stateButton->setFixedHeight(static_cast<int>(unitHeight));
        stateButtons.push_back(stateButton);
        statusBtnsLayout->addWidget(stateButton, Qt::AlignCenter);
    }

    statusButtonLayout->addLayout(statusBtnsLayout,0,1,Qt::AlignCenter);
    mainLayout->addLayout(statusButtonLayout);

    QFrame * statusBorder = new QFrame(this);
    statusBorder->setLineWidth(2);
    statusBorder->setMidLineWidth(1);
    statusBorder->setFrameShape(QFrame::HLine);
    statusBorder->setFrameShadow(QFrame::Raised);
    mainLayout->addWidget(statusBorder);

    if (conf->controlSpecs.size() > 0){
        controlsLayout = new QHBoxLayout;
        currLabel = new QLabel("CONTROLS: ");
        currLabel->setStyleSheet("font:"+labelFont+"pt;");
        controlsLayout->addWidget(currLabel,Qt::AlignLeft);

        for (uint i = 0; i < conf->controlSpecs.size(); i++){
            controlSpec * currSpec = conf->controlSpecs.at(i);
            if (currSpec->slider){
                QVBoxLayout * sliderLayout = new QVBoxLayout;
                currLabel = new QLabel(QString::fromStdString(currSpec->name));
                currLabel->setAlignment(Qt::AlignCenter);
                currLabel->setStyleSheet("font:"+labelFont+"pt;");
                sliderLayout->addWidget(currLabel);
                sliderControl = new QSlider(Qt::Horizontal);
                sliderControl->setRange(currSpec->minslider,currSpec->maxslider);
                QPalette palSlider = sliderControl->palette();
                sliderControl->setMaximumWidth(unitWidth*5);
                sliderLayout->addWidget(sliderControl);
                sliderLayout->setAlignment(Qt::AlignCenter);
                controlsLayout->addLayout(sliderLayout,Qt::AlignCenter);
                controlSliders.push_back(sliderControl);
                sliderCtrls.push_back(currSpec);
                connect(sliderControl, SIGNAL(sliderReleased()), this, SLOT(sliderValChanged()));
                connect(sliderControl, SIGNAL(actionTriggered(int)), this, SLOT(sliderValChanged(int)));
            }
            if (currSpec->button){
                buttonControl =new QPushButton();
                buttonControl->setText(QString::fromStdString(currSpec->name));
                QPalette palindi = buttonControl->palette();
                palindi.setColor(QPalette::Button, QColor(142,45,197).darker());
                buttonControl->setPalette(palindi);
                buttonControl->setAutoFillBackground(true);
                buttonControl->setStyleSheet("font:"+butLabelFont+"pt;");
                buttonControl->setFixedWidth(static_cast<int>(unitWidth));
                buttonControl->setFixedHeight(static_cast<int>(unitHeight*1.5));
                controlButtons.push_back(buttonControl);
                buttonCtrls.push_back(currSpec);
                controlsLayout->addWidget(buttonControl,Qt::AlignCenter);
                connect(buttonControl, SIGNAL(pressed()), this, SLOT(ctrlButtonPressed()));
                connect(buttonControl, SIGNAL(released()), this, SLOT(ctrlButtonReleased()));
            }
            if (currSpec->textField){
                QVBoxLayout * fieldLayout = new QVBoxLayout;
                currLabel = new QLabel(QString::fromStdString(currSpec->name));
                currLabel->setAlignment(Qt::AlignCenter);
                currLabel->setStyleSheet("font:"+labelFont+"pt;");
                fieldLayout->addWidget(currLabel);
                editControl = new QLineEdit;
                editControl->setValidator( new QIntValidator(0, 999999999, this) );
                editControl->setMaximumWidth(unitWidth*5);
                editControl->setStyleSheet("font:"+labelFont+"pt;");
                thisEdit=editControl;


                fieldLayout->addWidget(editControl);

                fieldLayout->setAlignment(Qt::AlignCenter);
                controlEdits.push_back(editControl);
                editCtrls.push_back(currSpec);
                controlsLayout->addLayout(fieldLayout,Qt::AlignCenter);
                connect(editControl, SIGNAL(returnPressed()), this, SLOT(editUpdated()));
            }
        }
        mainLayout->addLayout(controlsLayout);

        QFrame * controlBorder = new QFrame(this);
        controlBorder->setLineWidth(2);
        controlBorder->setMidLineWidth(1);
        controlBorder->setFrameShape(QFrame::HLine);
        controlBorder->setFrameShadow(QFrame::Raised);
        mainLayout->addWidget(controlBorder);
    }

    QString footerFont = QString::fromStdString(to_string(stringSize*3));

    QLabel * modeLabel = new QLabel;
    modeLabel->setAlignment(Qt::AlignCenter);
    modeLabel->setStyleSheet("font:"+footerFont+"pt;");
    if (conf->systemMode == RUN) modeLabel->setText("Mode: RUN");
    else if (conf->systemMode == TEST) modeLabel->setText("Mode: TEST");
    else modeLabel->setText("Mode: UNSPECIFIED");
    mainLayout->addWidget(modeLabel, Qt::AlignCenter);

    clock = new QLabel;
    clock->setAlignment(Qt::AlignCenter);
    clock->setStyleSheet("font:"+footerFont+"pt;");
    clock->setText("00:00:00");
    mainLayout->addWidget(clock, Qt::AlignCenter);
}

/*
*the clock seen on the display
*/
void MainWindow::updateClock(){
    int time = conf->dataCtrl->systemTimer->elapsed();
    time = time/1000;
    int sec = time%60;
    int min = (time/60)%60;
    int hrs = (time/3600)%60;
    string str_sec;
    string str_min;
    string str_hrs;
    if (sec < 10) str_sec = "0" + to_string(sec);
    else str_sec = to_string(sec);
    if (min < 10) str_min = "0" + to_string(min);
    else str_min = to_string(min);
    if (hrs < 10) str_hrs = "0" + to_string(hrs);
    else str_hrs = to_string(hrs);
    string updatedTime = str_hrs + ":" + str_min + ":" + str_sec;
    clock->setText(QString::fromStdString(updatedTime));
}

/*
* still trying to figure out this slider ones purposes
*/
void MainWindow::sliderValChanged(){
    QObject* obj = sender();
    for (uint i = 0; i < controlSliders.size(); i++){
        if (obj == controlSliders.at(i)){
            int value = controlSliders.at(i)->value();
            emit sendControlValue(value, sliderCtrls.at(i));
        }
    }
}

/*
*
*/
void MainWindow::sliderValChanged(int val){
    if (val == QAbstractSlider::SliderPageStepAdd || val == QAbstractSlider::SliderPageStepSub){
        QObject* obj = sender();
        for (uint i = 0; i < controlSliders.size(); i++){
            if (obj == controlSliders.at(i)){
                val = controlSliders.at(i)->value();
                emit sendControlValue(val, sliderCtrls.at(i));
            }

        }
    }
}

/*
* there are a few of the ones below that are unused but have to be edited
* out in other moudules
*/
void MainWindow::ctrlButtonPressed(){
    QObject* obj = sender();
    for (uint i = 0; i < controlButtons.size(); i++){
        if (obj == controlButtons.at(i)){
            emit sendControlValue(buttonCtrls.at(i)->pressVal, buttonCtrls.at(i));
        }
    }
}

void MainWindow::ctrlButtonReleased(){
    QObject* obj = sender();
    for (uint i = 0; i < controlButtons.size(); i++){
        if (obj == controlButtons.at(i)){
            emit sendControlValue(buttonCtrls.at(i)->releaseVal, buttonCtrls.at(i));
        }
    }
}

void MainWindow::editUpdated(){
    QObject* obj = sender();
    for (uint i = 0; i < controlEdits.size(); i++){
        if (obj == controlEdits.at(i)){
            QLineEdit * edit = static_cast<QLineEdit *>(obj);
            int val = stoi(edit->text().toStdString());
            emit sendControlValue(val, editCtrls.at(i));
            edit->setText("");
        }
    }
}

void MainWindow::activateStateMW(system_state * nextState){
    for(uint i = 0; i < stateButtons.size(); i++){
        if (stateButtons.at(i)->text().toStdString().compare(nextState->name) == 0){
            QPalette palplot = stateButtons.at(i)->palette();
            palplot.setColor(QPalette::Button, QColor(0,100,0));
            stateButtons.at(i)->setPalette(palplot);
        }
    }
}

void MainWindow::deactivateStateMW(system_state * prevState){
    for(uint i = 0; i < stateButtons.size(); i++){
        if (stateButtons.at(i)->text().toStdString().compare(prevState->name) == 0){
            QPalette palplot = stateButtons.at(i)->palette();
            palplot.setColor(QPalette::Button, QColor(70,70,70));
            stateButtons.at(i)->setPalette(palplot);
        }
    }
}
/*
* changes the text in a the specified textbox
*/
void MainWindow::drawEdit(QLineEdit * edit,QString dataDisplay ){
    edit= new QLineEdit();
    edit->setText(dataDisplay);
    edit->setMinimumWidth(unitWidth);
    edit->setStyleSheet("font:24pt;");
}
/*
* slot used to log all sub module messages
*/
void MainWindow::receiveMsg(string msg){
    logMessage(QString::fromStdString(msg));
}
/*
* used to show all sensors under group when tag is pushed
*/
void MainWindow::detailButtonPushed(){
    QObject* obj = sender();
    for (auto const &x: conf->groupMap){
        if (obj == detailButtons[x.first]) emit openDetailWindow(x.second);
    }
}

/*
* slot used to log all sub module messages
*yes its the same as the one above
*/
void MainWindow::receiveErrMsg(string msg){
    QString str = QString::fromStdString(msg);
    logMessage(QString::fromStdString(msg));
}


void MainWindow::updateHealth(){
    for (auto const &x: conf->groupMap){
        if(x.second->error){
            QPalette palb = healthButtons[x.first]->palette();
            palb.setColor(QPalette::Button, QColor(100,0,0));
            healthButtons[x.first]->setPalette(palb);
        } else {
            QPalette palb = healthButtons[x.first]->palette();
            palb.setColor(QPalette::Button, QColor(0,0,75));
            healthButtons[x.first]->setPalette(palb);
        }
    }
}

void MainWindow::updateFSM_MW(statemachine * currFSM){
    for (uint i = 0; i < conf->FSMs.size(); i++){
        statemachine * currMachine = conf->FSMs.at(i);
        if (currMachine == currFSM){
            for (uint j = 0; j < currMachine->states.size(); j++){
                if (currMachine->states.at(j)->active){
                    FSMButtons.at(i)->setText(QString::fromStdString(currMachine->states.at(j)->name));
                    QPalette palb = FSMButtons.at(i)->palette();
                    palb.setColor(QPalette::Button, QColor(0,100,0));
                    FSMButtons.at(i)->setPalette(palb);
                    return;
                }
            }
        }
    }
}
/*
* logs messages and the time it was recieved
*/
void MainWindow::logMessage(QString eMessage){
    if (message->count() >= 500) message->takeItem(0);
    string time = conf->dataCtrl->getProgramTime();
    string msg = eMessage.toStdString();
    msg = time + ": " + msg;
    message->addItem(QString::fromStdString(msg));
    message->scrollToBottom();
    dbtool->add_row_sec("logs","id,log",std::to_string(message->count())+",\" " +msg +" \"");
}

/*
* opens a new tab for specified groups
*/
void MainWindow::openDetailWindow(Group * grp){
    detailWindow= new detailPage();
    detailWindow->setConfObject(conf);
    detailWindow->setCurrentSystem(grp);
    tabs->addTab(detailWindow->central, QString::fromStdString(grp->groupId + "_Detail"));
    tabs->setCurrentIndex(tabs->count() - 1);
}

/*
* slot that closes tab on user input if its the first
* tab closes program
*/
void MainWindow::closeTab(int tabId){
    if ((tabId != 0) && (tabId != 1)) tabs->removeTab(tabId);
    else if (tabId == 0) shutdownSystem();
    tabs->setCurrentIndex(0);
}

/**
 * @brief MainWindow::passive_dialog creates a passive dialog box displaying
 *  the respective message. User response is not required
 * @param msg message to be displayed on the dialog box
 * @return 1 upon succesful run
 */
int MainWindow::passive_dialog(string msg){
    QDialog dlg;
    QVBoxLayout la(&dlg);
    QLabel ed;
    QString LabelFont = QString::number(stringSize*1.5);
    ed.setStyleSheet("font:"+LabelFont+"pt;");
    ed.setText(QString::fromStdString(msg));
    la.addWidget(&ed);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->setStyleSheet("font:"+LabelFont+"pt;");

    connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(close()));

    la.addWidget(buttonBox);
    dlg.setLayout(&la);
    dlg.adjustSize();

    dlg.exec();
    return 1;
}

/**
 * @brief MainWindow::active_dialog creates an interactive dialog on which the
 *  user can either accept or reject a request
 * @param msg message to be displayed on the dialog
 * @return either accepted or rejected
 */
int MainWindow::active_dialog(string msg){
    QDialog dlg;
    QVBoxLayout la(&dlg);
    QHBoxLayout la0;
    QLabel ed;
    QLabel ed0;
    QPixmap pixmap("warning.png");
    pixmap.setDevicePixelRatio(20);
    ed0.setPixmap(pixmap);
    ed.setText(QString::fromStdString(msg));
    QString LabelFont = QString::number(stringSize*1.5);
    ed.setStyleSheet("font:"+LabelFont+"pt;");
    la0.addWidget(&ed0);
    la0.addWidget(&ed);
    la.addLayout(&la0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setStyleSheet("font:"+LabelFont+"pt;");

    connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));

    la.addWidget(buttonBox);
    dlg.setLayout(&la);
    int result = dlg.exec();

    if(result == QDialog::Accepted){
        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

/*
* shutdown the system after prompting the user
*/
void MainWindow::shutdownSystem(){
    int confirmation = active_dialog("Are you sure you want to exit?");
    if (confirmation == QDialog::Accepted){
        int wdogpid = 0;
        fstream  wdpidfile;
        wdpidfile.open("watchdogpid.txt", ios::out | ios::in );
        wdpidfile >> wdogpid;
        wdpidfile.close();
        cout << "Killing watchdog with pid :" << wdogpid << endl;
        kill((pid_t)wdogpid,SIGKILL);
        this->close();
    }
}

/**
 * @brief MainWindow::updateEdits - updates text edit fields
 */
void MainWindow::updateEdits(meta * sensor){
    double num = sensor->calVal;
    ostringstream streamObj;
    streamObj << fixed;
    streamObj << setprecision(sensor->precision);
    streamObj << num;
    string val = streamObj.str();
    editTimers[sensor->sensorIndex]->start(sensor->checkRate);
    string field = val + " " + sensor->unit;
    edits[sensor->sensorIndex]->setText(QString::fromStdString(field));
}

/**
 * @brief MainWindow::checkTimeout - checks whether any lineEdit hasn't received updates
 */
void MainWindow::checkTimeout(){
    for (auto const& x : editTimers){
        if (!x.second->isActive()) edits[x.first]->setText("--");
    }
}

/*
* changes the color of sensors
*used to show overheating or major cooling
*/
void MainWindow::changeEditColor(string color, meta * sensor){
    if(color.compare("red") == 0){
        edits[sensor->sensorIndex]->setStyleSheet("color: #FF0000; font:"+editFont+"pt;");
    } else if(color.compare("blue") == 0){
        edits[sensor->sensorIndex]->setStyleSheet("color: #1E90FF; font:"+editFont+"pt;");
    } else if(color.compare("yellow") == 0){
        edits[sensor->sensorIndex]->setStyleSheet("color: #FFFF00; font:"+editFont+"pt;");
    }
}
