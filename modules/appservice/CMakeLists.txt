qt_add_library(appservice
    AppService.cpp
    AppService.h
)

target_link_libraries(appservice
    PUBLIC
        Qt6::Core
        modbuscontroller
        mqttworker
)
