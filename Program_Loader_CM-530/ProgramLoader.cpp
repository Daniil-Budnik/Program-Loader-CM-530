/**
    * @copyright   Copyright © 2024-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#include "ProgramLoader.h"
#include "ui_ProgramLoader.h"

ProgramLoader::ProgramLoader(QWidget* parent) : QMainWindow(parent), ui(new Ui::ProgramLoader){
    ui->setupUi(this);
    serial = new QSerialPort(this);
    regedit = new QSettings("SSTU", "Program_Loader_CM-530");

    messageWindow = new QMessageBox(this);
    messageWindow->setWindowTitle("CM-530");


    timerCOM.setInterval(1);
    connect(&timerCOM, SIGNAL(timeout()), this, SLOT(readCOM()));
    timerCOM.start();

    timerOut.setInterval(100);
    connect(&timerOut, SIGNAL(timeout()), this, SLOT(timerOutUpdate()));

    timerCOM_ForLoader.setInterval(1);
    connect(&timerCOM_ForLoader, SIGNAL(timeout()),
            this, SLOT(pushCOM_LoaderCommand()));

    connect(ui->B_CON, &QPushButton::clicked, this, &ProgramLoader::connectCOM);
    connect(ui->B_UPD, &QPushButton::clicked, this, &ProgramLoader::updateCOM);
    connect(ui->B_LOAD, &QPushButton::clicked, this, &ProgramLoader::loadCOM);
    updateCOM();
}

ProgramLoader::~ProgramLoader(){
    delete ui;
}

QStringList ProgramLoader::listDevice(){
    QStringList ARR;
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()){
        ARR.append(info.portName() + " : " + info.description());
    }
    return ARR;
}

void ProgramLoader::connectCOM() const{
    // Проверка, если порт уже работает, отключить
    if (serial->isOpen()){
        serial->close();
        ui->B_CON->setIcon(QIcon(QPixmap(":/Button/Connect.png")));
        Log::Logger::log(Log::DebugLevel::Info, __FUNCTION__, "Disconnected COM");
        ui->L_DVS->setEnabled(true);
        ui->B_LOAD->setEnabled(false);
        return;
    }

    QString COM = ui->L_DVS->currentText(), NamePort = "";

    // Разбираем название COM порта
    for (int i = 0; i < COM.length(); i++){
        if (COM[i] == ' ') break;
        NamePort += COM[i];
    }

    Log::Logger::log(Log::DebugLevel::Debug, __FUNCTION__, "PORT : " + NamePort);

    // Настройки виртуального COM порт устройства
    serial->setBaudRate(QSerialPort::Baud57600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setPortName(NamePort);

    // Производим соединение в режиме чтения и записи
    if (serial->open(QSerialPort::ReadWrite)){
        serial->setBaudRate(QSerialPort::Baud57600);
        ui->B_CON->setIcon(QIcon(QPixmap(":/Button/Disconnect.png")));
        ui->L_DVS->setEnabled(false);
        ui->B_LOAD->setEnabled(true);
        Log::Logger::log(Log::DebugLevel::Info, __FUNCTION__, "Connected COM");
        return;
    }
    Log::Logger::log(Log::DebugLevel::Error, __FUNCTION__, "Connection failed");
    Log::Logger::log(Log::DebugLevel::Error, __FUNCTION__, serial->errorString());
}

void ProgramLoader::updateCOM() const{
    ui->L_DVS->clear();
    if (QStringList devices = listDevice(); devices.isEmpty())
        ui->L_DVS->addItem("No matching devices found...");
    else for (const QString& device : devices) ui->L_DVS->addItem(device);
}

void ProgramLoader::loadCOM(){
    if (!serial->isOpen()) return;
    timerCOM_ForLoader.stop();
    Point_1_Check = false;
    Point_2_Check = false;

    QString path = "";
    if (!regedit->value("LastPath").toString().isEmpty())
        path = regedit->value("LastPath").toString();

    path = QFileDialog::getOpenFileName(nullptr,
                                        "Upload HEX file", path, "(*.bin)");

    if (path.isEmpty()){
        Log::Logger::log(Log::DebugLevel::Error, __FUNCTION__, "Path is empty!");
        return;
    }
    regedit->setValue("LastPath", path);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        Log::Logger::log(Log::DebugLevel::Error, __FUNCTION__, "Open file failed!");
        return;
    }

    hex = file.readAll();
    if (hex.isEmpty()){
        Log::Logger::log(Log::DebugLevel::Error, __FUNCTION__, "HEX file failed!");
        return;
    }

    loadEnabled = true;
    connect(this, &ProgramLoader::loadModeEnabled, this, &ProgramLoader::loadMode);
    timerOutCounter = 0;
    timerOutLevel = 10;
    timerOut.start();
    timerCOM_ForLoader.start();
    messageWindow->close();
    messageWindow->setIcon(QMessageBox::NoIcon);
    messageWindow->setText("Turn off and on the power of the СM-530.\n"
        "Do not disconnect the USB cable and do not disable the COM port.");
    messageWindow->setStandardButtons(QMessageBox::NoButton);
    messageWindow->setWindowModality(Qt::ApplicationModal);
    messageWindow->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
    messageWindow->show();
}

void ProgramLoader::readCOM(){
    if (!serial->isOpen()) return;
    const QString COM = serial->readAll();
    if (COM.isEmpty() and messageCOM.isEmpty()) return;
    if (COM.isEmpty()){
        if (Point_1_Check){
            if (messageCOM.indexOf("Ready..") != -1){
                timerOut.stop();
                Log::Logger::log(Log::DebugLevel::Info, __FUNCTION__, "CM-530: Load begin!");
                Point_1_Check = false;
                loadHEX();
            }
        }

        else if (Point_2_Check){
            if (messageCOM.indexOf("Checksum:") != -1){
                timerOut.stop();
                Log::Logger::log(Log::DebugLevel::Info, __FUNCTION__, "CM-530: Load finish!");
                Point_2_Check = false;
                loadGO();
            }
        }

        else{
            messageCOM = "";
            if (loadEnabled){
                loadEnabled = false;
                emit loadModeEnabled();
            }
        }
    }

    //Log::Logger::log(Log::DebugLevel::Message, __FUNCTION__, "Message:\n" + messageCOM);

    messageCOM += COM;
}

void ProgramLoader::loadMode(){
    disconnect(this, &ProgramLoader::loadModeEnabled, this, &ProgramLoader::loadMode);
    timerCOM_ForLoader.stop();
    messageWindow->close();
    messageWindow->setIcon(QMessageBox::NoIcon);
    messageWindow->setText("Loading. Please wait...");
    messageWindow->show();
    Point_1_Check = true;
    serial->write("\nL\n");
}

void ProgramLoader::loadHEX(){
    if (!serial->isOpen()) return;
    Point_2_Check = true;
    timerOutCounter = 0;
    timerOutLevel = 5;
    timerOut.start();
    serial->write(hex);
}

void ProgramLoader::loadGO() const{
    serial->write("\nGO\n");
    messageWindow->setIcon(QMessageBox::NoIcon);
    messageWindow->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    messageWindow->setText("Firmware loading into CM-530 completed successfully.");
    messageWindow->setStandardButtons(QMessageBox::Ok);
    messageWindow->show();
}

void ProgramLoader::pushCOM_LoaderCommand() const{
    if (!serial->isOpen()) return;
    serial->write("#");
}

void ProgramLoader::timerOutUpdate(){
    timerOutCounter += 1;
    if (timerOutCounter >= timerOutLevel*10){
        timerOutCounter = 0;
        timerOut.stop();
        messageWindow->close();
        messageWindow->setIcon(QMessageBox::Critical);
        messageWindow->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        messageWindow->setText("Error in COM port operation during firmware loading.\n"
            "Restart the device and the program and try again...");
        messageWindow->setStandardButtons(QMessageBox::Ok);
        messageWindow->show();
    }
}
