qt_add_library(modbuscontroller
    ModbusController.cpp
    ModbusController.h
)

target_include_directories(modbuscontroller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-c/include
        ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-cpp/include
)

target_link_directories(modbuscontroller PUBLIC
    ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-cpp/lib
)

target_link_libraries(modbuscontroller
    PUBLIC
        Qt6::Core
        Qt6::SerialBus
        messagequeue
        paho-mqttpp3-static
        paho-mqtt3a-static
        rpcrt4
        crypt32
)
