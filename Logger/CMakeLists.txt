# Copyright © 2021-2025 Daniil Budnik. All rights reserved.
# Author: Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>

cmake_minimum_required(VERSION 3.10)

# Проект
project(Logger VERSION 1.0 LANGUAGES CXX)
message(STATUS ">>> Build dependencies: Logger")

# Настройки
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Импорт стандартных Qt библиотек
find_package(${QT} REQUIRED COMPONENTS Core)

# Подключаем файлы к проекту
set(PROJECT_SOURCES
        Logger.cpp
        Logger.h
        Logger_global.h
)

# Подключаем файлы к библиотеке
add_library(Logger SHARED ${PROJECT_SOURCES})

# Подключаем библиотеки
target_link_libraries(Logger PRIVATE ${QT}::Core DbgHelp)

# Название библиотеки
target_compile_definitions(Logger PRIVATE LOGGER_LIBRARY)

# Установка папки для исполняемых файлов
set(CMAKE_COMPILE_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}/DLL_Soft/DLL_Logger")

# Устанавливаем папку для исполняемого файла
set_target_properties(Logger PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_COMPILE_DIRECTORY}")
set_target_properties(Logger PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_COMPILE_DIRECTORY}")
set_target_properties(Logger PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_COMPILE_DIRECTORY}")
