if (NOT EXISTS "${CMAKE_SOURCE_DIR}/3rd/benchmark/CMakeLists.txt")
    message(FATAL_ERROR "The git submodule for google benchmark has not been downloaded!")
endif ()


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/benchmark")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" "${CMAKE_SOURCE_DIR}/3rd/benchmark" -DGOOGLETEST_PATH=${GTEST_ROOT}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/benchmark"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/benchmark"
)
set(GBENCH_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rd/benchmark/include")
set(GBENCH_LIBS "${CMAKE_BINARY_DIR}/3rd/benchmark/src/libbenchmark.a;${CMAKE_BINARY_DIR}/3rd/benchmark/src/libbenchmark_main.a")
