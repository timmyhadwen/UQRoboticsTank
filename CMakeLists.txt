cmake_minimum_required(VERSION 3.1)
project(WifiFreeRTOS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c90")

set(SOURCE_FILES
    main.c)

cmake_minimum_required(VERSION 3.1)

project(my_test)

add_custom_target(my_test COMMAND make -C ${mytest_SOURCE_DIR}
                         CLION_EXE_DIR=${PROJECT_BINARY_DIR})