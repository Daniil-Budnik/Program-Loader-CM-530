/**
    * @copyright   Copyright © 2021-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#ifndef LOGGER_H
#define LOGGER_H

#pragma once
#pragma warning(disable : 4996)

#include <QFile>
#include <QDir>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QRunnable>
#include <QThreadPool>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include "Logger_global.h"

namespace Log{
    class LOGGER_EXPORT Logger;
    class LOGGER_EXPORT LoggerWorker;
    class LOGGER_EXPORT LoggerTaskQueue;

    /** @enum DebugLevel 
        @brief Выбор режима отладки сообщений.*/
    enum class DebugLevel {
        Message = 0,                    /**< @details Стандартное сообщение */
        Warning = 1,                    /**< @details Предупреждение / Не критическая ошибка */
        Error = 2,                      /**< @details Обычная ошибка */
        Fatal = 3,                      /**< @details Критическая ошибка */
        Info = 4,                       /**< @details Информационное сообщение */
        TODO = 5,                       /**< @details Предупреждение, что данный блок нужно исправить */
        Debug = 6,                      /**< @details Отладочная информация */
        Marker = 7,                     /**< @details Вспоготалеьный маркер */
        System = 8,                     /**< @details Системная информация */
        Soft = 9,                       /**< @details Вызов ядра программы */
        Progress = 10                   /**< @details Прогресс бар */
    };

    const int META_ID_DEBUG_LEVEL = qRegisterMetaType<DebugLevel>("DebugLevel");
    
}

/**
  * @file       Logger.h
  * @brief      Система логирования программного обеспечения.
  * @{
*/

/**
 * Список задач для логирования:
 *  @todo 1) Необходимо провести тест работы скорости сохранения.
 *  @todo 2) Ограничить кол-во лог файлов, сделать опциональным.
 *  @todo 3) При необходимости создать параллельный поток для быстродействия.
 *  @todo 4) Проверить внешнее воздействие на работоспособность программы.
 *  @todo 5) Сделать возможным читать спец символы ANSI в .log файлах, или отключить их совсем.
 */

/**
 * @brief Класс для работы с системой логирования программного обеспечения.
 * @details Хранит в себе набор статических методов и настроек для удобного и оптимального\n
            введения лог файла программного обеспечения.
 * @warning Метод целиком и полностью использует статические методы.
*/
class LOGGER_EXPORT Log::Logger final : public QObject {

    Q_OBJECT

public:

    /** Последний маркер логирования. */
    static DebugLevel level_old;

    /** Старт системы логирования.
     * @param name Название программного обеспечения
     * @param version Версия программного обеспечения.
     */
    static void initSoftware(const QString& name = "", const QString& version = "");

    /** Логирование.
     * @param level Тип сообщения.
     * @param func Метод, который инициализировал сообщение.
     * @param message Сообщение.
     */
    static void log(const DebugLevel& level, const QString& func, const QVariant& message);

    /** Логирование со своей временной меткой.
     * @param level Тип сообщения.
     * @param func Метод, который инициализировал сообщение.
     * @param message Сообщение.
     * @param time Временная метка.
     */
    static void log(const DebugLevel& level, const QString& func, const QVariant& message, QTime time);

    /** Пошаговое логирование.
     * @param level Тип сообщения.
     * @param func Метод, который инициализировал сообщение.
     * @param current Номер текущей итерации.
     * @param full Предполагаемое кол-во итераций.
     * @param message Сообщение.
     */
    static void log(const DebugLevel& level, const QString& func,
        int current, int full, const QVariant& message);

    /** Простое отладочное сообщение.
     * @param message Отладочное сообщение.
     * @param func Метод, инициализировавший сообщение.
     */
    static void debug(const QVariant& message, const QString& func = "");

    /** Консольный прогресс бар.
     * @param value Текущее значение.
     * @param min Минимальное значение.
     * @param max Максимальное значение.
     * @param begin Время начала для расчёта оставшегося время загрузки
     */
    static void progress_value(float value, float min, float max, const QTime& begin = QTime());

    /** Консольный прогресс бар.
     * @param current_value Текущее значение в процентах. [0..100]
     * @param begin Время начала для расчёта оставшегося время загрузки
     */
    static void progress(float current_value, const QTime& begin = QTime());

    /** Метод позволяет удалить последний выведенный лог. */
    static void removeOneLog();

    /** Проверка работы отладчика. */
    static void test();

    static bool visibleFunction;            /**< Включение вывода функции. */
    static bool visibleKey;                 /**< Включение вывода тима сообщения. */
    static bool visibleColor;               /**< Включение вывода цвета. */
    static bool visibleTime;                /**< Включение вывода времени. */

    /** Включение быстрого режима, путём потери частых логов.
        @warning В этом режиме возможна потеря некоторых логов. */
    static bool fastControl;

private:

    explicit Logger() = default;
    ~Logger() override = default;

    /** Логирование в файл. */
    static std::fstream logFile;
    static int countLine;
    static bool locker;
    static bool init;
    static QMutex *mutex;
};

/**
 * @brief Технический класс для создания очереди для работы логирования.
 * @details Позволяет создать задачу для последующей установки в очередь.
 * @warning Не требуется для использования где-либо помимо текущего файла.
*/
class LOGGER_EXPORT Log::LoggerWorker final : public QRunnable {

public:

    /** Универсальная задача для логирования. */
    using Task = std::function<void()>;

    /** Конструктор. */
    explicit LoggerWorker(Task task);

protected:

    /** Задача для выполнения. */
    Task task;

    /** Реализация выполнения задачи. */
    void run() override;
};

/**
 * @brief Технический класс реализации очереди выполнения логирования в параллельном потоке.
 * @details Позволяет при выключенном режиме [Logger::fastControl] не терять логи во время работы.
 * @warning Не требуется для использования где-либо помимо текущего файла.
*/
class LOGGER_EXPORT Log::LoggerTaskQueue final : public QObject {

    Q_OBJECT
public:

    /** Реализация статической очереди. */
    static LoggerTaskQueue& instance();

    /** Установка задачи в очередь. */
    void enqueueTask(const LoggerWorker::Task& task);

protected:

    /** Конструктор. */
    LoggerTaskQueue();

    /** Процесс выполнения задачи. */
    void processTasks();

private:
    QMutex mutex;
    QQueue<LoggerWorker::Task> tasks;
    QThreadPool threadPool;
};

// Логирование с временной меткой
#define LOG(level, func, message) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::log(level, func, message); \
    })

// Логирование с временной меткой QTime
#define LOG_WITH_TIME(level, func, message, time) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::log(level, func, message, time); \
    })

// Пошаговое логирование
#define LOG_STEP(level, func, current, full, message) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::log(level, func, current, full, message); \
    })

// Простое отладочное сообщение
#define DEBUG(message, func) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::debug(message, func); \
    })

// Консольный прогресс бар со значениями
#define PROGRESS_VALUE(value, min, max, begin) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::progress_value(value, min, max, begin); \
    })

// Консольный прогресс бар с процентным значением
#define PROGRESS(current_value, begin) \
    Log::LoggerTaskQueue::instance().enqueueTask([=]() { \
        Log::Logger::progress(current_value, begin); \
    })

// Удаление последнего лога
#define REMOVE_ONE_LOG() \
    Log::LoggerTaskQueue::instance().enqueueTask([]() { \
        Log::Logger::removeOneLog(); \
    })

// Проверка работы отладчика
#define TEST() \
    Log::LoggerTaskQueue::instance().enqueueTask([]() { \
        Log::Logger::test(); \
    })

#endif //LOGGER_H
/** @} */
