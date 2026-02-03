cmake_minimum_required(VERSION 3.16)

project(IoTGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)
find_package(Qt6 REQUIRED COMPONENTS Core Quick SerialBus)

add_subdirectory(modules/appservice)
add_subdirectory(modules/messagequeue)
add_subdirectory(modules/mqttworker)
add_subdirectory(modules/modbuscontroller)
add_subdirectory(modules/types)

qt_add_resources(APP_RESOURCES
    resources.qrc
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${APP_RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Quick
        appservice
)
