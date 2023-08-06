cmake_minimum_required(VERSION 3.26)
project(CTales VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
        main.c
)

target_compile_features(CTales PRIVATE c_std_11)

target_compile_options(CTales PRIVATE
        $<$<CONFIG:Release>:-Ofast -march=native -flto -funroll-loops>
)

find_package(Curses REQUIRED)
find_path(JANSSON_INCLUDE_DIR NAMES jansson.h)
find_library(JANSSON_LIBRARY NAMES jansson)
find_library(MENU_LIBRARY NAMES menu)
find_path(MENU_INCLUDE_DIR NAMES menu.h)
find_path(MIMALLOC_INCLUDE_DIR NAMES mimalloc.h)
find_library(MIMALLOC_LIBRARY NAMES mimalloc)

target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIR} ${JANSSON_INCLUDE_DIR} ${MENU_INCLUDE_DIR} ${MIMALLOC_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES} ${JANSSON_LIBRARY} ${MENU_LIBRARY} ${MIMALLOC_LIBRARY})
