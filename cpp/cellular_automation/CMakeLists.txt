cmake_minimum_required(VERSION 3.18)
project(WireWorld2D)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/home/zander/Qt/6.2.1/gcc_64")

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(WireWorld2D main.cpp MainWindow.cpp MainWindow.h FieldWidget.cpp
        FieldWidget.h WireWorld.cpp WireWorld.h
        VectorField.h Game.h Field.h WireWorldQt.h GameLife.cpp
        GameLife.h GameLifeQt.h GameQt.h Factory.h GameLifeQt.cpp WireWorldQt.cpp GamesIDs.h ParserRLE.h)
target_link_libraries(WireWorld2D Qt::Core Qt::Gui Qt::Widgets)
