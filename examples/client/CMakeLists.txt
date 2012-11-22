set(client_SOURCES ${modules_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/client.c
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

