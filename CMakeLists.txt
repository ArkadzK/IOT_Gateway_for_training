cmake_minimum_required(VERSION 3.16)

project(ModbusMaster VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
            Quick
            Core
            SerialBus
)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appModbusMaster
            main.cpp
)

qt_add_qml_module(appModbusMaster
            URI ModbusMaster
            VERSION 1.0
            QML_FILES
                Main.qml
            SOURCES
                modbuscontroller.h
                modbuscontroller.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appModbusMaster PROPERTIES
    # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appModbusMaster
    # MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    # MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    # MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appModbusMaster
    PRIVATE
        Qt6::Quick
        Qt6::Core
        Qt6::SerialBus
)

include(GNUInstallDirs)
install(TARGETS appModbusMaster
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
