/**
    * @copyright   Copyright © 2024-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#include <QApplication>
#include <QDate>
#include "Logger.h"
#include "ProgramLoader.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    Log::Logger::initSoftware("[SSTU] Program Loader CM-530 [Author: Daniil Budnik]", "Version: 1.0 " +
                              QDate::currentDate().toString("dd/MM/yyyy"));
    auto* loader = new ProgramLoader();
    loader->show();
    return QApplication::exec();
}
