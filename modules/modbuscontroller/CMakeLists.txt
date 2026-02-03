qt_add_library(modbuscontroller
    ModbusController.cpp
    ModbusController.h
)

target_include_directories(modbuscontroller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(modbuscontroller
    PUBLIC
        Qt6::Core
        Qt6::SerialBus
        messagequeue
    PRIVATE
        types
)
