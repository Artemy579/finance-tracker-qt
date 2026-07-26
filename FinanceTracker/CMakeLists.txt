cmake_minimum_required(VERSION 3.20)
project(FinanceTracker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(finance_tracker
    src/main.cpp
    src/Database.cpp
    src/Database.h
    src/ExpenseChartWidget.cpp
    src/ExpenseChartWidget.h
    src/MainWindow.cpp
    src/MainWindow.h
)

target_include_directories(finance_tracker PRIVATE src)
target_link_libraries(finance_tracker PRIVATE Qt6::Widgets Qt6::Sql)
qt_finalize_executable(finance_tracker)
