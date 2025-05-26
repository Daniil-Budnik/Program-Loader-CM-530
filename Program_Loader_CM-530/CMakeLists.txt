# Copyright © 2024-2025 Daniil Budnik. All rights reserved.
# Author: Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>


cmake_minimum_required(VERSION 3.31)
project(Program_Loader_CM-530)

# Настройки
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_ENABLE_HIGHDPI_SCALING ON)

find_package(${QT} REQUIRED COMPONENTS
        Core
        Widgets
        SerialPort
)


set(SOURCE_FILE
        main.cpp
        ProgramLoader.cpp
        ProgramLoader.h

        ../Image/Image.qrc
        execIcon.rc
)

add_executable(Program_Loader_CM-530 ${SOURCE_FILE})


target_link_libraries(Program_Loader_CM-530 PRIVATE
        ${QT}::Core
        ${QT}::Widgets
        ${QT}::SerialPort
        DbgHelp
)

set_target_properties(Program_Loader_CM-530 PROPERTIES WIN32_EXECUTABLE TRUE)

# --------------------------------------------------------------------------------------------------------------------------------
# Подключаем Logger.dll
# --------------------------------------------------------------------------------------------------------------------------------
#add_subdirectory("../Logger")
add_dependencies(Program_Loader_CM-530 Logger)
include_directories("../Logger")
target_link_libraries(Program_Loader_CM-530 PUBLIC Logger)

# Перекидываем Logger.dll после сборки
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Logger.dll
        COMMAND ${CMAKE_COMMAND} -E copy  $<TARGET_FILE:Logger> ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "---> ${NAME_EXE}.exe -> Add Logger.dll"
        DEPENDS Logger
)

# Создаём цель, связанную с кастомной командой
add_custom_target(Logger_DLL_COPY_Program_Loader_CM_530
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Logger.dll
)

# Делаем зависимость на действие копирования
add_dependencies(Program_Loader_CM-530 Logger_DLL_COPY_Program_Loader_CM_530)
# --------------------------------------------------------------------------------------------------------------------------------