include(FindPkgConfig)
pkg_search_module(CHECK REQUIRED check)

include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})
link_libraries(${PROJECT_NAME} ${CHECK_LIBRARIES})

add_executable(basics basics.c)

add_test(NAME Basics COMMAND basics)
