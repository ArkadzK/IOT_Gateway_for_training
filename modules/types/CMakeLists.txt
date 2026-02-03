qt_add_library(types STATIC
    ModbusTypes.h
)

target_link_libraries(types
    PUBLIC
    Qt6::Core
)

target_include_directories(types
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
