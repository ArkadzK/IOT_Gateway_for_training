cmake_minimum_required(VERSION 3.16)
project(IoTGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к модулям
add_subdirectory(modules/messagequeue)
add_subdirectory(modules/mqttworker)
add_subdirectory(modules/modbuscontroller)

# Основное приложение
add_executable(${PROJECT_NAME}
    main.cpp
)

# Линкуем модули
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        messagequeue
        mqttworker
        modbuscontroller
)
