/**
    * @copyright   Copyright © 2024-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#ifndef PROGRAMLOADER_H
#define PROGRAMLOADER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QSettings>
#include <QPointer>
#include <QTimer>
#include "Logger.h"

QT_BEGIN_NAMESPACE

namespace Ui{
    class ProgramLoader;
}

QT_END_NAMESPACE

class ProgramLoader final : public QMainWindow{
    Q_OBJECT

public:
    explicit ProgramLoader(QWidget* parent = nullptr);
    ~ProgramLoader() override;
    static QStringList listDevice();

signals:
    void loadModeEnabled();

protected slots:
    void connectCOM() const;
    void updateCOM() const;
    void loadCOM();
    void readCOM();
    void loadMode();
    void loadHEX();
    void loadGO() const;
    void pushCOM_LoaderCommand() const;
    void timerOutUpdate();

private:
    Ui::ProgramLoader* ui = nullptr;
    QMessageBox* messageWindow = nullptr;
    QPointer<QSettings> regedit = nullptr;
    QPointer<QSerialPort> serial = nullptr;
    QString messageCOM = "";
    QString portName = "COM1";
    QTimer timerCOM, timerCOM_ForLoader;
    bool loadEnabled = false;
    bool Point_1_Check = false;
    bool Point_2_Check = false;
    QByteArray hex;

    QTimer timerOut;
    int timerOutCounter = 0;
    int timerOutLevel = 0;
};


#endif
