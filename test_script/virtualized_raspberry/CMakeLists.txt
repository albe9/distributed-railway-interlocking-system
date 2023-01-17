cmake_minimum_required(VERSION 3.0.0)
project(Virtualized_raspberry VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(Virtualized_raspberry main.c)

include_directories(includes)

add_library(init src/initTask.c)
add_library(wifi src/wifiTask.c)
add_library(global src/global_variables.c)

target_link_libraries(Virtualized_raspberry PUBLIC init wifi global)


target_include_directories(Virtualized_raspberry PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/includes"
                          )          

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
