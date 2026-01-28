add_library(messagequeue
    MessageQueue.cpp
    MessageQueue.h
)

target_include_directories(messagequeue
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
