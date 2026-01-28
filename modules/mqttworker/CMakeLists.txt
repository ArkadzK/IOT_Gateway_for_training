add_library(mqttworker
    MqttWorker.cpp
    MqttWorker.h
)

target_include_directories(mqttworker
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        /c/Study/QT_traning/ExtLibs/paho-mqtt-cpp/include
)

target_link_directories(mqttworker PUBLIC
    /c/Study/QT_traning/ExtLibs/paho-mqtt-cpp/lib
)

target_link_libraries(mqttworker
    PUBLIC
        messagequeue
        paho-mqttpp3
        paho-mqtt3a
)

