#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utilitydb.h"
#include "parse.h"
#include "schedule.h"

#include <algorithm>
#include <QCompleter>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    parser = new Parse();
    fileDownloader = new FileDownloader(QUrl("https://asu.pnu.edu.ua/data/groups-list.js"), this);
    connect(fileDownloader, SIGNAL(downloaded()), this, SLOT(loadAllGroups()));


    // todo: remove before PR
    UtilityDB* utilityDb = new UtilityDB();

    qDebug() << utilityDb -> isConnected();

    utilityDb->dropTable("СОІ-32");
    utilityDb->dropTable("KN-35");

    utilityDb->createScheduleTable("СОІ-32");
    utilityDb->createScheduleTable("KN-35");

    qDebug() << utilityDb -> doesTableExist("СОІ-32");
    qDebug() << utilityDb -> doesTableExist("KN-35");

    //utilityDb->createScheduleTable("KN_31");
    /*
    // Some little example
    UtilityDB* utilityDb = new UtilityDB();
    //utilityDb->dropTable("KN_31");


    //utilityDb->createScheduleTable("KN_31");

    Schedule schedule;
    schedule.groupName = "KN-31";

    QList<ScheduleList> *groupSchedule = new QList<ScheduleList>();
    ScheduleList scheduleList;
    scheduleList.date = "22.05.2001";
    scheduleList.nameOfDay = "monday";
    scheduleList.numOfCouple = 2;
    scheduleList.timeStapOfCouple = "12:00 - 13:20";
    scheduleList.coupleDesc =  "C++";
    groupSchedule->push_back(scheduleList);
    schedule.groupSchedule = groupSchedule;
    utilityDb -> insertScheduleToTable("KN_31", schedule);
    Schedule schedule1 = utilityDb ->getScheduleByTableName("KN_31");
    Schedule schedule2 = utilityDb ->getScheduleByTableNameInRange("KN_31", QDate(2001, 3, 22), QDate(2002, 3, 22));
    qDebug() << utilityDb -> doesTableExist("KN_31");
    */
    //Parse *parse = new Parse();
    //Schedule newList = parse->parseSchedule("https://asu.pnu.edu.ua/static/groups/1002/1002-0732.html");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::loadAllGroups()
{
    QByteArray response = fileDownloader->getDownloadedData();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString jsFileContent = codec->toUnicode(response);
    groups = parser->parseJSFileWithAllGroups(jsFileContent);
    QStringList groupNames;
    std::for_each(groups.begin(), groups.end(), [this, &groupNames](UniversityGroup group) {
        ui->allGroupsComboBox->addItem(group.name);
        groupNames << group.name;
    });
    QCompleter *comboBoxCompleter = new QCompleter(groupNames, this);
    comboBoxCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->allGroupsComboBox->setCompleter(comboBoxCompleter);
}

void MainWindow::on_getScheduleButton_clicked()
{
   auto iterObject = std::find_if(groups.begin(), groups.end(),
                                [this](UniversityGroup group){ return group.name == ui->allGroupsComboBox->currentText().trimmed();});

   if (iterObject != groups.end()) {
       QString groupUnitCode = QString::number(iterObject->unitCode);
       int amountOfDigitsInMaxGroupNumber = QString::number(groups.length()).length();
       QString groupSchedulelink = "https://asu.pnu.edu.ua/static/groups/" + groupUnitCode + '/' + groupUnitCode + '-'
               + QStringLiteral("%1").arg(iterObject->sequenceNumber, amountOfDigitsInMaxGroupNumber, 10, QLatin1Char('0')) + ".html";

       Schedule testScheduleList = parser->parseSchedule(groupSchedulelink);
       UtilityDB db;
       if(db.doesTableExist(testScheduleList.groupName)){
           qDebug() << "EXIST";
           //НЕ РОЗУМІЮ ЯК ДІСТАТИ ДАТУ З ЦЬОГО ПІКЕРА ЙОБАНОГО НУ НАХУЙ Я ПІШОВ ЗАКРИВАТИ ЛАБИ З ПАРАЛЕЛЬНОГО
           //db.getScheduleByTableNameInRange(testScheduleList.groupName, , );
       }
       else{
           db.createScheduleTable("English");
           db.insertScheduleToTable(testScheduleList.groupName, testScheduleList);
       }
   }
   else {
       QMessageBox::critical(this, "Error", "There is no such group at the university!");
   }
}


void MainWindow::on_startDateCalendarWidget_clicked(const QDate &date) { startFilterDate = date; }

void MainWindow::on_endDateCalendarWidget_clicked(const QDate &date) { endFilterDate = date; }

