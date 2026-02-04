qt_add_library(appservice
    AppService.cpp
    AppService.h
)

target_include_directories(appservice
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(appservice
    PUBLIC
        Qt6::Core
        modbuscontroller
        mqttworker
    PRIVATE
        types
)
