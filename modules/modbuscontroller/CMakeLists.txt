qt_add_library(modbuscontroller
    ModbusController.cpp
    ModbusController.h
)

target_link_libraries(modbuscontroller
    PUBLIC
        Qt6::Core
        Qt6::SerialBus
        messagequeue
)
