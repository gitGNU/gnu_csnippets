set(client_SOURCES ${client_SOURCES}
	${CMAKE_CURRENT_LIST_DIR}/client.c
)

add_executable(client EXCLUDE_FROM_ALL ${client_SOURCES})
target_link_libraries(client ${this_library})

