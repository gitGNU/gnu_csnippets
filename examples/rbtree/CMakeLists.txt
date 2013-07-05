set(rbtree_SOURCES ${CMAKE_CURRENT_LIST_DIR}/rbtree.c)
add_executable(rbtree EXCLUDE_FROM_ALL ${rbtree_SOURCES})
target_link_libraries(rbtree -L../build -lcsnippets ${csnippets_LIBRARIES})

