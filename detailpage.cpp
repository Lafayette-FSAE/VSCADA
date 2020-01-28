#include "detailpage.h"
#include "ui_detailpage.h"

detailPage::detailPage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::detailPage)
{

    central = new QWidget();
    mainLayout = new QVBoxLayout();

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    central->setLayout(mainLayout);

    QRect rec = QApplication::desktop()->screenGeometry();
    int height=rec.height();
    int width=rec.width();

    unitWidth=width/20;//100
    unitHeight=height/20;//56

    stringSize = unitWidth/10;//10
    editFont = QString::number(stringSize*2);
    fontSize = QString::number(stringSize);

}

detailPage::~detailPage()
{
    delete ui;
}
/*
*
*/
void detailPage::update(){
    vector<meta *> allSensors = currentGroup->get_mainsensors();

    for(uint i = 0; i < allSensors.size(); i++){
        lineEdit = new QLineEdit;
        checkTmr = new QTimer;
        connect(checkTmr, SIGNAL(timeout()), checkTmr, SLOT(stop()));
        connect(checkTmr, SIGNAL(timeout()), this, SLOT(checkTimeout()));
        checkTmr->start(allSensors.at(i)->checkRate);
        edits.push_back(lineEdit);
        lineEdit->setStyleSheet("font: 20pt; color: #FFFF00");
        lineEdit->setAlignment(Qt::AlignCenter);
        lineEdit->setDisabled(1);
        editTimers.push_back(checkTmr);
    }

    QRect rec = QApplication::desktop()->screenGeometry();
    int screenWidth = rec.width();

    int fieldColCount = 0;
    int fieldRowCount = 0;

    vector<Group *> grps;
    grps = conf->subsystems;

    QGridLayout * subsystemSectionLayout = new QGridLayout;

    vector<meta *> subMeta = currentGroup->get_mainsensors();

    QLabel * headerLabel = new QLabel;
    headerLabel->setFixedWidth(static_cast<int>(unitWidth*1.5));
    headerLabel->setFixedHeight(unitHeight*2);
    headerLabel->setText(QString::fromStdString(currentGroup->groupId));
    QString  subLabelFont = QString::number(stringSize*3);
    headerLabel->setStyleSheet("font:"+subLabelFont+"pt;");
    headerLabel->setFixedWidth(screenWidth - 50);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel,Qt::AlignCenter);

    if(subMeta.size() > 0){
        for (uint j = 0; j < subMeta.size(); j++){
            if (fieldColCount >= 12){
                fieldRowCount ++;
                fieldColCount = 0;
            }

            for (uint k = 0; k < allSensors.size(); k++){
                if (allSensors.at(k) == subMeta.at(j)){
                    QLabel * label = new QLabel;
                    label->setText(QString::fromStdString(subMeta.at(j)->sensorName));
                    label->setFixedWidth(unitWidth*2);
                    label->setFixedHeight(static_cast<int>(unitHeight*0.8));
                    QString LabelFont = QString::number(stringSize*2);
                    label->setStyleSheet("font:"+LabelFont+"pt;");
                    subsystemSectionLayout->addWidget(label,fieldRowCount,fieldColCount);

                    QLineEdit * edit = edits.at(k);
                    edit->setStyleSheet("font:"+editFont+"pt;");
                    edit->setFixedWidth(static_cast<int>(unitWidth*2.5));
                    edit->setFixedHeight(static_cast<int>(unitHeight*0.8));
                    subsystemSectionLayout->addWidget(edit,fieldRowCount,fieldColCount+1);
                    fieldColCount += 2;
                }
            }

            if(fieldColCount != 11 && j != subMeta.size()-1){
                QFrame * hBorder1 = new QFrame(this);
                hBorder1->setLineWidth(2);
                hBorder1->setMidLineWidth(1);
                hBorder1->setFrameShape(QFrame::VLine);
                hBorder1->setFrameShadow(QFrame::Raised);
                subsystemSectionLayout->addWidget(hBorder1, fieldRowCount,fieldColCount);
            }
            fieldColCount++;
        }
    }

    QWidget * subsystemWidget = new QWidget;
    subsystemWidget->setLayout(subsystemSectionLayout);
    subsystemWidget->adjustSize();

    QScrollArea * subsystemArea = new QScrollArea;
    subsystemArea->setFixedWidth(screenWidth-30);
    subsystemArea->setFixedHeight(screenWidth/7);
    subsystemArea->setBackgroundRole(QPalette::Window);
    subsystemArea->setWidget(subsystemWidget);

    mainLayout->addWidget(subsystemArea);
    QFrame * hBorder1 = new QFrame(this);
    hBorder1->setLineWidth(2);
    hBorder1->setMidLineWidth(1);
    hBorder1->setFrameShape(QFrame::HLine);
    hBorder1->setFrameShadow(QFrame::Raised);
    mainLayout->addWidget(hBorder1, Qt::AlignCenter);

    QLineEdit * input=new QLineEdit;
    input->setStyleSheet("font:"+editFont+"pt;");
    input->setFixedWidth(static_cast<int>(unitWidth*2.5));
    input->setFixedHeight(static_cast<int>(unitHeight*0.8));
    mainLayout->addWidget(input);

    checkbox = new QCheckBox("Characterise", this);
    checkbox->setTristate(currentGroup->isCharcterised);
    connect(checkbox, SIGNAL(stateChanged(int)),currentGroup,SLOT(createGroupTable()));
    mainLayout->addWidget(checkbox);
}

/*
*
*/
void detailPage::setConfObject(Config * config){
    this->conf = config;
    connect(conf->dataCtrl, SIGNAL(updateDisplay(meta *)), this, SLOT(updateEdits(meta *)));
    connect(conf->dataCtrl, SIGNAL(updateEditColor(string, meta *)), this, SLOT(changeEditColor(string, meta *)));
}

/*
*
*/
void detailPage::setCurrentSystem(Group * subsystem){
    currentGroup = subsystem;
    update();
    vector<meta*> allSensors = currentGroup->get_mainsensors();
    for(uint i = 0; i < allSensors.size(); i++){
        allSensors.at(i)->state = 0;
    }
}


/**
 * @brief detailPage::updateEdits - updates text edit fields
 */
void detailPage::updateEdits(meta * currSensor){
    vector<meta*> allSensors = currentGroup->get_mainsensors();
    for(uint i = 0; i < edits.size(); i++){
        if(allSensors.at(i) == currSensor){
            double num = currSensor->calVal;
            ostringstream streamObj;
            streamObj << fixed;
            streamObj << setprecision(2);
            streamObj << num;
            string val = streamObj.str();
            editTimers.at(i)->start(currSensor->checkRate);
            string field = val + " " + currSensor->unit;
            edits.at(i)->setText(QString::fromStdString(field));
        }
    }
}

/**
 * @brief detailPage::checkTimeout - checks whether any lineEdit hasn't received updates
 */
void detailPage::checkTimeout(){
    for(uint i = 0; i < edits.size(); i++){
        if (!editTimers.at(i)->isActive()) edits.at(i)->setText("--");
    }
}

/*
*
*/
void detailPage::changeEditColor(string color, meta * sensor){
    vector<meta*> allSensors = currentGroup->get_mainsensors();
    for(uint i = 0; i < edits.size(); i++){
        if (allSensors.at(i) == sensor) {
            if(color.compare("red") == 0){
                edits.at(i)->setStyleSheet("color: #FF0000; font:"+editFont+"pt;");
            } else if(color.compare("blue") == 0){
                edits.at(i)->setStyleSheet("color: #1E90FF; font:"+editFont+"pt;");
            } else if(color.compare("yellow") == 0){
                edits.at(i)->setStyleSheet("color: #FFFF00; font:"+editFont+"pt;");
            }
        }
    }
}
