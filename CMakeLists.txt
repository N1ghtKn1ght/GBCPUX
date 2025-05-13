cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME GBCPUX)

project(GBCPUX VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick OpenGL OpenGLWidgets)

file(GLOB_RECURSE SOURCES
    source/*.cpp
)

file(GLOB_RECURSE HEADERS
    source/*.h
)

include_directories(${PROJECT_NAME} PUBLIC
    source/
    source/hardware/
    source/hardware/tools/
    source/tools/
)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION ${HEADERS} ${SOURCES}
)

#set_target_properties(${PROJECT_NAME}
   #PROPERTIES WIN32_EXECUTABLE TRUE
 #  QT_QMLCACHEGEN_DIRECT_CALLS ON
#)

qt_add_resources(${PROJECT_NAME} "resources"
    PREFIX "/"
    FILES
        assets/logo.ico
)

qt_add_qml_module(${PROJECT_NAME}
    URI "GBCPUX"
    VERSION 1.0
    QML_FILES 
        ui/Main.qml
    DEPENDENCIES QtQuick QtCore
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL32)
