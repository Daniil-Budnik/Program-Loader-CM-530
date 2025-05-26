#include "Logger.h"

// Проверка работы отладчика
#define THREAD_CONTROL \
    if (fastControl){ if (!mutex->tryLock()) return; } \
    else{ while (!mutex->tryLock()){} } \
    QMutexLocker locker(mutex);

bool Log::Logger::init = false; /**< Инициализация разрешена только один раз. */
bool Log::Logger::visibleColor = true; /**< Включение вывода функции. */
bool Log::Logger::visibleFunction = true; /**< Включение вывода тима сообщения. */
bool Log::Logger::visibleKey = true; /**< Включение вывода цвета. */
bool Log::Logger::visibleTime = true; /**< Включение вывода времени. */
bool Log::Logger::fastControl = false; /**< Включение быстрого режима, путём потери частых логов. */
int Log::Logger::countLine = 0; /**< Кол-во строк для последнего лога. */

std::fstream Log::Logger::logFile = std::fstream(); /**< Поток для сохранения логов. */
Log::DebugLevel Log::Logger::level_old = DebugLevel::System; /**< Предыдущий маркер лога. */
QMutex* Log::Logger::mutex = new QMutex(QMutex::Recursive); /**< Блокировка повторного вызова. */

#ifdef _WIN32
#include <windows.h>
#include <csignal>
#include <dbghelp.h>

inline bool enable_ansi_escape_codes(){
    if (GetStdHandle(STD_OUTPUT_HANDLE) == INVALID_HANDLE_VALUE) return false;

    DWORD dwMode = 0;
    if (!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode)) return false;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode)) return false;

    return true;
}

void signalHandler(const int signal){
    void* stack[100];
    const HANDLE handleProcess = GetCurrentProcess();

    SymInitialize(handleProcess, nullptr, TRUE);
    const unsigned short frames = CaptureStackBackTrace(0, 100, stack, nullptr);

    auto* symbol = static_cast<SYMBOL_INFO*>(calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;

    const QDir dir;
    // ReSharper disable once CppExpressionWithoutSideEffects
    dir.mkpath("crashes");

    if (QFile crashReport("crashes/crash_report_" +
            QDateTime::currentDateTime().toString("dd_MM_yyyy_hh-mm-ss") + ".txt");
        crashReport.open(QIODevice::WriteOnly)){
        QString errorString = "Received signal " + QString::number(signal) + "\n";
        for (unsigned short i = 0; i < frames; i++){
            SymFromAddr(handleProcess, reinterpret_cast<DWORD64>(stack[i]), nullptr, symbol);
            errorString += "Function: " + QString(symbol->Name) +
                "() at address: 0x" + QString::number(symbol->Address, 16) + "\n";
        }
        crashReport.write(errorString.toUtf8());
        crashReport.close();
    }

    free(symbol);
    exit(signal);
}

// crashReport
inline void crashReportEnabled(){
    std::signal(SIGINT, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGFPE, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGBREAK, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGABRT_COMPAT, signalHandler);
}
#else
inline bool enable_ansi_escape_codes(){}
void signalHandler(const int signal){}
inline void crashReportEnabled(){}
#endif

void Log::Logger::initSoftware(const QString& name, const QString& version){
    THREAD_CONTROL

    if (init){
        log(DebugLevel::Warning, __FUNCTION__, "Double init software!!!");
        return;
    }

    // Включаем обработку спец символов
    enable_ansi_escape_codes();

    // Включаем краш репорт
    crashReportEnabled();

    // Создаём папку для логов
    const QDir dir;
    // ReSharper disable once CppExpressionWithoutSideEffects
    dir.mkpath("log");

    // Указываем файл лога
    logFile.open("log/" +
                 QDateTime::currentDateTime().toString("dd_MM_yyyy_hh-mm-ss").toStdString() + ".log",
                 std::ofstream::out | std::ios::app);

    // Стартовое сообщение
    std::cout << "[" + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss:zzzzzz").toStdString() + "] ";
    if (!name.isEmpty()) std::cout << name.toStdString();
    if (!version.isEmpty()){
        if (!name.isEmpty()) std::cout << " -> ";
        std::cout << version.toStdString();
    }
    std::cout << std::endl;

    // Включаем все настройки
    visibleColor = true;
    visibleFunction = true;
    visibleKey = true;
    visibleTime = true;
    init = true;

    mutex->unlock();
}

void Log::Logger::log(const DebugLevel& level, const QString& func, const QVariant& message){
    log(level, func, message.toString(), QTime::currentTime());
}

void Log::Logger::log(const DebugLevel& level, const QString& func, const QVariant& message, const QTime time){
    THREAD_CONTROL

#ifdef DEBUG_BUILD
#else
    switch (level){
        case DebugLevel::Debug: return;
        case DebugLevel::Marker: return;
        case DebugLevel::TODO: return;
        default:;
    }
#endif

    QString mess = message.toString();
    mess.replace("\r\n", "\n");
    countLine = mess.count('\n') + mess.count('\r') + 1 +
        mess.count(QChar::ParagraphSeparator) + mess.count(QChar::LineSeparator);
    while (mess.count("  ") > 0) mess.replace("  ", " ");

    // Переменные
    QString TIME, COLOR, KEY;

    // Определяем временную метку
    if (visibleTime) TIME = "[" + time.toString("hh:mm:ss:zzzzzz") + "]";

    // Определяем настройки
    switch (level){
    case DebugLevel::Message:
        COLOR = "\x1b[0m";
        KEY = "MESSAGE";
        break;
    case DebugLevel::Warning:
        COLOR = "\x1b[33m";
        KEY = "WARNING";
        break;
    case DebugLevel::Error:
        COLOR = "\x1b[31m";
        KEY = "ERROR";
        break;
    case DebugLevel::Fatal:
        COLOR = "\x1b[41m";
        KEY = "CRITICAL";
        break;
    case DebugLevel::Debug:
        COLOR = "\x1b[32m";
        KEY = "DEBUG";
        break;
    case DebugLevel::Info:
        COLOR = "\x1b[35m";
        KEY = "INFO";
        break;
    case DebugLevel::TODO:
        COLOR = "\x1b[43m\x1b[37m\x1b[1m";
        KEY = "TODO";
        break;
    case DebugLevel::Soft:
        COLOR = "\x1b[36m";
        KEY = "SOFT";
        break;
    case DebugLevel::Marker:
        COLOR = "\x1b[42m";
        KEY = "MARKER";
        break;
    case DebugLevel::System:
        COLOR = "\x1b[46m";
        KEY = "SYSTEM";
        break;
    case DebugLevel::Progress:
        COLOR = "\x1b[33m";
        KEY = "LOAD";
        break;
    default:
        COLOR = "\x1b[0m";
        KEY = "MESSAGE";
        break;
    }

    if (!logFile.is_open()) std::cerr << "LOG ERROR: not open file." << std::endl;


    // Корректировка
    logFile << std::flush;
    logFile.sync();

    // Настройка цвета
    if (visibleColor){
        std::cout << COLOR.toStdString();
        logFile << COLOR.toStdString();
    }

    // Вывод временной метки
    if (visibleTime){
        std::cout << std::setw(17) << TIME.toStdString();
        logFile << std::setw(17) << TIME.toStdString();
    }

    // Вывод типа сообщения
    if (visibleKey){
        std::cout << std::setw(11) << KEY.toStdString() << ": ";
        logFile << std::setw(11) << KEY.toStdString() << ": ";
    }

    // Вывод функции, где произошёл вызов метода
    if (visibleFunction and !func.isEmpty()){
        std::cout << std::setw(40) << func.toStdString() << std::setw(3) << " | ";
        logFile << std::setw(40) << func.toStdString() << std::setw(3) << " | ";
    }

    std::cout << mess.toStdString() << std::endl << "\x1b[0m";
    level_old = level;
    if (DebugLevel::Progress == level) return;
    logFile << mess.toStdString() << std::endl << "\x1b[0m";
    logFile.flush();

    if (logFile.fail()) std::cerr << "LOG ERROR: fail save log." << std::endl;


    mutex->unlock();
}

void Log::Logger::log(const DebugLevel& level, const QString& func,
                      const int current, const int full, const QVariant& message){
    const QString msg = "[" + QString::number(current) + "/" + QString::number(full) + "] " + message.toString();
    log(level, func, msg);
}

void Log::Logger::debug(const QVariant& message, const QString& func){
    log(DebugLevel::Debug, func, message);
}

void Log::Logger::progress_value(const float value, const float min, const float max, const QTime& begin){
    const float progress_value = std::round(((value - min) * 100 / (max - min)) * 100.0) / 100.0;
    progress(progress_value, begin);
}

void Log::Logger::progress(const float current_value, const QTime& begin){
    THREAD_CONTROL

    if (level_old == DebugLevel::Progress){
        for (int id = 0; id < countLine; id++) std::cout << "\x1b[1A\x1b[K";
    }

    countLine = 1;
    level_old = DebugLevel::Progress;

    const QTime TIME_NOW = QTime::currentTime();
    const QString COLOR = "\x1b[33m";
    const QString KEY = "LOAD";
    const QString TIME = "[" + TIME_NOW.toString("hh:mm:ss:zzzzzz") + "]";

    std::cout << COLOR.toStdString();
    std::cout << std::setw(17) << TIME.toStdString();
    std::cout << std::setw(11) << KEY.toStdString() << ": ";

    constexpr int ELEMENTS_PROGRESS = 38;
    const float VALUE = max(0, min(100, current_value));
    const float POINT = VALUE * static_cast<float>(ELEMENTS_PROGRESS) / 100.0f;
    QString prog = "";
    for (int id = 0; id < ELEMENTS_PROGRESS; id++){
        if (id < POINT) prog += "#";
        else prog += " ";
    }

    std::cout << std::setw(ELEMENTS_PROGRESS + 2) << "[" + prog.toStdString() + "]" << std::setw(3) << " | ";
    std::cout << std::setw(10) << QString::number(VALUE).toStdString() + "% ";
    if (begin != QTime()){
        const QTime dT = QTime(0, 0, 0).addMSecs(begin.msecsTo(TIME_NOW));
        const QTime t = QTime(0, 0, 0).addMSecs((dT.msecsSinceStartOfDay() * (100 - VALUE)) / VALUE);
        std::cout << " -> " + t.toString("hh:mm:ss").toStdString();
    }
    std::cout << std::endl << "\x1b[0m";
    mutex->unlock();
}

void Log::Logger::removeOneLog(){
    THREAD_CONTROL
    for (int id = 0; id < countLine; id++) std::cout << "\x1b[1A\x1b[K";
    countLine = 0;
    mutex->unlock();
}

void Log::Logger::test(){
    log(DebugLevel::System, __FUNCTION__, "ALL PRINT LOG MESSAGE TEST");
    log(DebugLevel::Debug, "TEST", "TEST");
    log(DebugLevel::TODO, "TEST", "TEST");
    log(DebugLevel::Error, "TEST", "TEST");
    log(DebugLevel::Fatal, "TEST", "TEST");
    log(DebugLevel::Info, "TEST", "TEST");
    log(DebugLevel::Message, "TEST", "TEST");
    log(DebugLevel::Marker, "TEST", "TEST");
    log(DebugLevel::Soft, "TEST", "TEST");
    log(DebugLevel::Warning, "TEST", "TEST");
    countLine = 10;
}

Log::LoggerWorker::LoggerWorker(Task task) : task(std::move(task)){
}

void Log::LoggerWorker::run(){
    task();
}

Log::LoggerTaskQueue& Log::LoggerTaskQueue::instance(){
    static LoggerTaskQueue queue;
    return queue;
}

void Log::LoggerTaskQueue::enqueueTask(const LoggerWorker::Task& task){
    QMutexLocker locker(&mutex);
    tasks.enqueue(task);
    processTasks();
}

Log::LoggerTaskQueue::LoggerTaskQueue(){
    threadPool.setMaxThreadCount(8);
}

void Log::LoggerTaskQueue::processTasks(){
    while (!tasks.isEmpty() && threadPool.activeThreadCount() < threadPool.maxThreadCount()){
        const auto task = tasks.dequeue();
        threadPool.start(new LoggerWorker(task));
    }
}
