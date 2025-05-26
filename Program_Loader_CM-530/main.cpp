/**
    * @copyright   Copyright © 2024-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#include <QApplication>
#include <QDate>
#include "Logger.h"
#include "ProgramLoader.h"

const QString version = "1.1";

int main(int argc, char* argv[]){

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);
    Log::Logger::initSoftware("[SSTU] Program Loader CM-530 [Author: Daniil Budnik]", "Version: " +
                              version + " " + QDate::currentDate().toString("dd/MM/yyyy"));

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    app.setOrganizationName("SSTU");
    app.setOrganizationDomain("https://github.com/Daniil-Budnik/Program-Loader-CM-530");
    app.setApplicationName("Program Loader CM-530x");
    app.setApplicationVersion(version);

    auto* loader = new ProgramLoader();
    loader->show();
    return QApplication::exec();
}
