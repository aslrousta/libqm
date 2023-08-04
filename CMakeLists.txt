cmake_minimum_required(VERSION 3.7.0)
project(qm
    VERSION 0.1.0
    DESCRIPTION "Quantitative Fixed-point (D36.18) Arithmetic Library"
    HOMEPAGE_URL "https://github.com/aslrousta/libqm"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/libqm.h)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_file(libqm.pc.in libqm.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libqm.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
