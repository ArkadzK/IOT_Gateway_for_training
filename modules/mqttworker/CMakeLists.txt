add_library(mqttworker
    MqttWorker.cpp
    MqttWorker.h
)

target_include_directories(mqttworker
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-c/include
        ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-cpp/include
)

target_link_directories(mqttworker PUBLIC
    ${PROJECT_SOURCE_DIR}/ExtLibs/paho-mqtt-cpp/lib
)

target_link_libraries(mqttworker
    PUBLIC
        Qt6::Core
        messagequeue

        paho-mqttpp3-static
        paho-mqtt3a-static

        ws2_32
        rpcrt4
        crypt32
)
