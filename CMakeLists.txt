# Copyright © 2024-2025 Daniil Budnik. All rights reserved.
# Author: Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>

cmake_minimum_required(VERSION 3.31)
project(CM-530)

# Предварительные настройки
set(QT Qt5)
set(PROJECT_DIR "..")
set(BASE_PROJECT "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}/")

# Обязательное использование кодировки UTF-8 для исходников
add_compile_options("/utf-8")

# Проверка типа сборки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
endif()

# (#) Собираем модуль системы отладки
add_subdirectory("Logger")

# (1) Собираем программу для программирования
add_subdirectory("Program_Loader_CM-530")